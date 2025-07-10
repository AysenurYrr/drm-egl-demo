#ifndef RENDERER_H
#define RENDERER_H

#include <gbm.h>
#include <EGL/egl.h>
#include "drm-common.h"

// shared render context definition
struct render_context {
    struct gbm_device *gbm;
    EGLDisplay dpy;
    EGLSurface surf;
    EGLContext ctx;
    struct gbm_surface *gs;
};

int setup_rendering(struct drm *drm, struct render_context *ctx);
void cleanup_rendering(struct render_context *ctx);

// New functions to initialize and draw a triangle
int init_triangle();
void draw_triangle();

#endif