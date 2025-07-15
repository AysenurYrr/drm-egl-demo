#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include "triangle.h"
#include "shader_utils.h"

// Store shader program and VBO as static variables
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


int init_triangle() {
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

void draw_triangle() {
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