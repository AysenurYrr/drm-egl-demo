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

/* ===== Triangle setup using GLSL ===== */

// Store shader program and VBO as static variables for this module
static GLuint triangle_program = 0;
static GLuint triangle_vbo = 0;

static const char *vertex_shader_src =
    "attribute vec3 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "}\n";

static const char *fragment_shader_src =
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

static GLuint compile_shader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

int init_triangle() {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
    if (0 == vertex_shader)
    {
        fprintf(stderr, "Failed to compile vertex shader\n");
        return -1;
    }
    else
    {
        fprintf(stderr, "Vertex shader compiled successfully\n");
    }

    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
    if (0 == fragment_shader) 
    {
        fprintf(stderr, "Failed to compile fragment shader\n");
        return -1;
    }
    else
    {
        fprintf(stderr, "Fragment shader compiled successfully\n");
    }

    triangle_program = glCreateProgram();
    glAttachShader(triangle_program, vertex_shader);
    glAttachShader(triangle_program, fragment_shader);
    glLinkProgram(triangle_program);

    GLint linked;
    glGetProgramiv(triangle_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(triangle_program, sizeof(log), NULL, log);
        fprintf(stderr, "Program link error: %s\n", log);
        glDeleteProgram(triangle_program);
        return -1;
    }
    // Shaders can be deleted after linking
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Set up triangle vertex data (3 vertices, 3 float components each)
    float vertices[] = {
         0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };

    glGenBuffers(1, &triangle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    return 0;
}

void draw_triangle() {
    // Clear screen with a dark gray color (optional)
    printf("Drawing triangle...\n");
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(triangle_program);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);

    GLint posAttrib = glGetAttribLocation(triangle_program, "aPos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(posAttrib);
}