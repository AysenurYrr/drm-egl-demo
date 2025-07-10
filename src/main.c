#include <stdio.h>
#include <unistd.h>
#include "drm-common.h"
#include "renderer.h"

#include <GL/gl.h>

static int init(struct drm *drm, struct render_context *ctx) {
    if (init_drm(drm, "/dev/dri/card0")) 
    {
        fprintf(stderr, "init_drm failed\n");
        return -1;
    } 
    else 
    {
        fprintf(stderr, "init_drm succeeded: fd=%d\n", drm->fd);
    }

    if (setup_rendering(drm, ctx)) 
    {
        fprintf(stderr, "setup_rendering failed\n");
        close(drm->fd);
        return -1;
    } 
    else 
    {
        fprintf(stderr, "setup_rendering succeeded: gbm=%p, dpy=%p, surf=%p, ctx=%p\n",
                ctx->gbm, ctx->dpy, ctx->surf, ctx->ctx);
    }

    // Initialize the triangle shaders and vertex data
    if (init_triangle()) 
    {
        fprintf(stderr, "init_triangle failed\n");
        return -1;
    }
    else 
    {
        fprintf(stderr, "init_triangle succeeded\n");
    }
    return 0;
}

static void draw(struct drm *drm, struct render_context *ctx) {
    // Render our triangle
    draw_triangle();

    // Swap the buffers (renders the triangle on screen)
    eglSwapBuffers(ctx->dpy, ctx->surf);

    // Optionally: if you need to interact with DRM (e.g. setting the CRTC),
    // you can lock the GBM buffer and set the mode. Otherwise, you can keep it simple.
    // DRM scanout: lock the buffer, get its fb and update the CRTC
    struct gbm_bo *bo = gbm_surface_lock_front_buffer(ctx->gs);
    if (NULL == bo) 
    {
        fprintf(stderr, "gbm_surface_lock_front_buffer failed\n");
        return;
    }
    else
    {
        fprintf(stderr, "gbm_surface_lock_front_buffer succeeded: bo=%p\n", bo);
    }
    struct drm_fb *fb = drm_fb_get_from_bo(bo);
    if (NULL == fb) 
    {
        fprintf(stderr, "drm_fb_get_from_bo failed\n");
        gbm_surface_release_buffer(ctx->gs, bo);
        return;
    }
    else
    {
        fprintf(stderr, "drm_fb_get_from_bo succeeded: fb_id=%u\n", fb->fb_id);
    }
    drmModeSetCrtc(drm->fd,
                   drm->crtc_id,
                   fb->fb_id,
                   0, 0,
                   &drm->connector_id, 1,
                   drm->mode);
    gbm_surface_release_buffer(ctx->gs, bo);
}

int main() {
    struct drm drm = {0};
    struct render_context ctx;

    if (init(&drm, &ctx)) 
    {
        return -1;
    }

    while (1) 
    {
        draw(&drm, &ctx);
        usleep(16000); // ~60 FPS
    }

    cleanup_rendering(&ctx);
    close(drm.fd);
    return 0;
}