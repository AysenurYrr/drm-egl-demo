#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include <GLES2/gl2.h>

GLuint compile_shader(GLenum type, const char *src);
int link_program(unsigned program);

#endif