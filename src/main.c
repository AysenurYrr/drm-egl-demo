#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/poll.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "drm-common.h"
#include "renderer.h"
#include "triangle.h"

static void page_flip_handler(int fd, unsigned int frame,
                              unsigned int sec, unsigned int usec,
                              void *data)
{
    int *waiting_for_flip = data;
    *waiting_for_flip = 0;
}

static int init(struct drm *drm, struct render_context *ctx) {
    if (init_drm(drm, "/dev/dri/card0")) {
        fprintf(stderr, "init_drm failed\n");
        return -1;
    }

    if (setup_rendering(drm, ctx)) {
        fprintf(stderr, "setup_rendering failed\n");
        close(drm->fd);
        return -1;
    }

    if (init_triangle()) {
        fprintf(stderr, "init_triangle failed\n");
        return -1;
    }
    return 0;
}

static void draw(struct drm *drm, struct render_context *ctx, struct gbm_bo **previous_bo, uint32_t *previous_fb) {
    draw_triangle();
    eglSwapBuffers(ctx->dpy, ctx->surf);

    struct gbm_bo *bo = gbm_surface_lock_front_buffer(ctx->gs);
    if (!bo) {
        fprintf(stderr, "gbm_surface_lock_front_buffer failed\n");
        return;
    }

    struct drm_fb *fb = drm_fb_get_from_bo(bo);
    if (!fb) {
        fprintf(stderr, "drm_fb_get_from_bo failed\n");
        gbm_surface_release_buffer(ctx->gs, bo);
        return;
    }

    if (*previous_fb == 0) {
        if (drmModeSetCrtc(drm->fd, drm->crtc_id, fb->fb_id,
                           0, 0, &drm->connector_id, 1, drm->mode)) {
            fprintf(stderr, "drmModeSetCrtc failed: %s\n", strerror(errno));
            gbm_surface_release_buffer(ctx->gs, bo);
            return;
        }
    } else {
        int waiting_for_flip = 1;
        if (drmModePageFlip(drm->fd, drm->crtc_id, fb->fb_id,
                            DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip)) {
            fprintf(stderr, "drmModePageFlip failed: %s\n", strerror(errno));
            gbm_surface_release_buffer(ctx->gs, bo);
            return;
        }

        struct pollfd pfd = {
            .fd = drm->fd,
            .events = POLLIN,
        };

        while (waiting_for_flip) {
            int ret = poll(&pfd, 1, -1);
            if (ret < 0) {
                perror("poll");
                break;
            }

            if (pfd.revents & POLLIN) {
                drmEventContext evctx = {
                    .version = DRM_EVENT_CONTEXT_VERSION,
                    .page_flip_handler = page_flip_handler,
                };
                drmHandleEvent(drm->fd, &evctx);
            }
        }
    }

    if (*previous_bo) {
        gbm_surface_release_buffer(ctx->gs, *previous_bo);
    }

    *previous_bo = bo;
    *previous_fb = fb->fb_id;
}

int main() {
    struct drm drm = {0};
    struct render_context ctx = {0};
    struct gbm_bo *previous_bo = NULL;
    uint32_t previous_fb = 0;

    if (init(&drm, &ctx)) {
        return -1;
    }

    while (1) {
        draw(&drm, &ctx, &previous_bo, &previous_fb);
        usleep(16000);
    }

    cleanup_rendering(&ctx);
    close(drm.fd);
    return 0;
}
