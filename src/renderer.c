#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gbm.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <xf86drmMode.h>
#include "drm-common.h"
#include "renderer.h"

int render_frame(struct drm *drm) {
    // 2) GBM device
    struct gbm_device *gbm = gbm_create_device(drm->fd);
    if (!gbm) { perror("gbm_create_device"); return -1; }

    // 3) EGL init on GBM
    EGLDisplay dpy = eglGetDisplay((EGLNativeDisplayType)gbm);
    if (dpy == EGL_NO_DISPLAY || !eglInitialize(dpy, NULL, NULL)) {
        fprintf(stderr, "eglInitialize failed\n");
        return -1;
    }
    eglBindAPI(EGL_OPENGL_ES_API);

    // 4) Choose config
    const EGLint cfg_attribs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    EGLConfig cfg; EGLint ncfg;
    if (!eglChooseConfig(dpy, cfg_attribs, &cfg, 1, &ncfg) || ncfg == 0) {
        fprintf(stderr, "eglChooseConfig failed\n");
        return -1;
    }

    // 5) Get native visual ID (format) for GBM
    EGLint vid;
    eglGetConfigAttrib(dpy, cfg, EGL_NATIVE_VISUAL_ID, &vid);

    // 6) Create GBM surface with that format
    struct gbm_surface *gs = gbm_surface_create(
        gbm,
        drm->mode->hdisplay, drm->mode->vdisplay,
        vid,                          // ⇐ EGL_NATIVE_VISUAL_ID
        GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING
    );
    if (!gs) {
        fprintf(stderr, "gbm_surface_create failed\n");
        return -1;
    }

    // 7) Wrap GBM surface as EGL window surface
    EGLSurface surf = eglCreateWindowSurface(dpy, cfg, gs, NULL);
    if (surf == EGL_NO_SURFACE) {
        fprintf(stderr, "eglCreateWindowSurface failed: 0x%04X\n", eglGetError());
        return -1;
    }

    // 8) Create & bind ES2 context
    const EGLint ctx_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctx_attribs);
    if (ctx == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext failed: 0x%04X\n", eglGetError());
        return -1;
    }
    if (!eglMakeCurrent(dpy, surf, surf, ctx)) {
        fprintf(stderr, "eglMakeCurrent failed: 0x%04X\n", eglGetError());
        return -1;
    }

    // 9) Render a single red frame
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(dpy, surf);

    // 10) Scanout: ilk frame’i CRTC’ye set et
    struct gbm_bo *bo = gbm_surface_lock_front_buffer(gs);
    struct drm_fb *fb = drm_fb_get_from_bo(bo);
    if (!fb) {
        fprintf(stderr, "drm_fb_get_from_bo failed\n");
        return -1;
    }
    drmModeSetCrtc(drm->fd,
                   drm->crtc_id,
                   fb->fb_id,
                   0, 0,
                   &drm->connector_id, 1,
                   drm->mode);
    gbm_surface_release_buffer(gs, bo);

    // 11) Görmek için 5 saniye bekle
    sleep(5);

    // 12) Temizlik
    eglDestroySurface(dpy, surf);
    gbm_surface_destroy(gs);
    eglDestroyContext(dpy, ctx);
    eglTerminate(dpy);
    gbm_device_destroy(gbm);
    return 0;
}