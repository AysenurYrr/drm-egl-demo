#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gbm.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <xf86drmMode.h>
#include "drm-common.h"
#include "renderer.h"

int setup_rendering(struct drm *drm, struct render_context *ctx) {
    ctx->gbm = gbm_create_device(drm->fd);
    if (!ctx->gbm) { perror("gbm_create_device"); return -1; }

    ctx->dpy = eglGetDisplay((EGLNativeDisplayType)ctx->gbm);
    if (ctx->dpy == EGL_NO_DISPLAY || !eglInitialize(ctx->dpy, NULL, NULL)) {
        fprintf(stderr, "eglInitialize failed\n");
        return -1;
    }
    eglBindAPI(EGL_OPENGL_ES_API);

    const EGLint cfg_attribs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    EGLConfig cfg; EGLint ncfg;
    if (!eglChooseConfig(ctx->dpy, cfg_attribs, &cfg, 1, &ncfg) || ncfg == 0) {
        fprintf(stderr, "eglChooseConfig failed\n");
        return -1;
    }

    EGLint vid;
    eglGetConfigAttrib(ctx->dpy, cfg, EGL_NATIVE_VISUAL_ID, &vid);

    ctx->gs = gbm_surface_create(
        ctx->gbm,
        drm->mode->hdisplay, drm->mode->vdisplay,
        vid,
        GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING
    );
    if (!ctx->gs) {
        fprintf(stderr, "gbm_surface_create failed\n");
        return -1;
    }

    ctx->surf = eglCreateWindowSurface(ctx->dpy, cfg, ctx->gs, NULL);
    if (ctx->surf == EGL_NO_SURFACE) {
        fprintf(stderr, "eglCreateWindowSurface failed: 0x%04X\n", eglGetError());
        return -1;
    }

    const EGLint ctx_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    ctx->ctx = eglCreateContext(ctx->dpy, cfg, EGL_NO_CONTEXT, ctx_attribs);
    if (ctx->ctx == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext failed: 0x%04X\n", eglGetError());
        return -1;
    }
    if (!eglMakeCurrent(ctx->dpy, ctx->surf, ctx->surf, ctx->ctx)) {
        fprintf(stderr, "eglMakeCurrent failed: 0x%04X\n", eglGetError());
        return -1;
    }

    return 0;
}

void cleanup_rendering(struct render_context *ctx) {
    eglDestroySurface(ctx->dpy, ctx->surf);
    gbm_surface_destroy(ctx->gs);
    eglDestroyContext(ctx->dpy, ctx->ctx);
    eglTerminate(ctx->dpy);
    gbm_device_destroy(ctx->gbm);
}