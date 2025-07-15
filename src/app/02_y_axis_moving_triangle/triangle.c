#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include "./triangle.h"
#include "shader_utils.h"
#include "../main.h" // Include screen_context definition

// Store shader program and VBO as static variables
static GLuint triangle_program = 0;
static GLuint triangle_vbo = 0;

static const char *vertex_shader_src =
    "uniform float u_y_offset;\n"
    "attribute vec4 a_vertex;\n"

    "void main() {\n"
    "    gl_Position = a_vertex + vec4(0.0, u_y_offset, 0.0, 0.0);\n"
    "}\n";

static const char *fragment_shader_src =
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "}\n";

float yOffset = 0.0f;
float ySpeed = 0.01f;
int direction = 1;

void update_y_offset() {
    yOffset += ySpeed * direction;
    if (yOffset > 0.5f || yOffset < -0.5f) {
        direction *= -1; // Reverse direction when reaching bounds
    }
}
int init_triangle(struct screen_context *screen_ctx) {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
    if (0 == vertex_shader) {
        fprintf(stderr, "Failed to compile vertex shader\n");
        return -1;
    }

    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
    if (0 == fragment_shader) {
        fprintf(stderr, "Failed to compile fragment shader\n");
        return -1;
    }

    triangle_program = glCreateProgram();
    glAttachShader(triangle_program, vertex_shader);
    glAttachShader(triangle_program, fragment_shader);
    glLinkProgram(triangle_program);

    // Link the program using link_program
    if (link_program(triangle_program) < 0) {
        fprintf(stderr, "Failed to link program\n");
        return -1;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

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

void draw_triangle(struct screen_context *screen_ctx) {

    update_y_offset(); // Update the yOffset before drawing

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(triangle_program);

    GLint yOffsetLocation = glGetUniformLocation(triangle_program, "u_y_offset");
    glUniform1f(yOffsetLocation, yOffset);

    GLint a_vertex_location = glGetAttribLocation(triangle_program, "a_vertex");
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glEnableVertexAttribArray(a_vertex_location);
    glVertexAttribPointer(a_vertex_location, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glViewport(0, 0, screen_ctx->screen_width, screen_ctx->screen_height);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(a_vertex_location);
}