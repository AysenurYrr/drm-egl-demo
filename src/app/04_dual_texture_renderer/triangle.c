// triangle.c
#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include "triangle.h"
#include "shader_utils.h"

// from main.c
extern int screen_width, screen_height;

// two-phase shader handles:
static GLuint triangle_program = 0;
static GLuint display_program  = 0;

// two triangle VBOs
static GLuint triangle_vbo[2];

// FBOs + textures
static GLuint fbo[3];
static GLuint tex[3];

// animation offsets
static float yOffset[2]   = {0.f, 0.f};
static float ySpeed[2]    = {0.01f, 0.02f};
static int   direction[2] = {1, 1};

// Simple tri shaders
static const char *vert_src =
  "uniform float u_y_offset;\n"
  "attribute vec4 a_vertex;\n"
  "void main() {\n"
  "  gl_Position = a_vertex + vec4(0.0, u_y_offset, 0.0, 0.0);\n"
  "}\n";
static const char *frag_src =
  "precision mediump float;\n"
  "void main() {\n"
  "  gl_FragColor = vec4(0.0,1.0,0.0,1.0);\n"
  "}\n";


// Display‐pass shaders
static const char *display_vert_src =
  "attribute vec2 a_texCoord;\n"
  "varying vec2 v_texCoord;\n"
  "void main() {\n"
  "  v_texCoord = a_texCoord;\n"
  "  gl_Position = vec4(2.0*a_texCoord - 1.0, 0.0, 1.0);\n"
  "}\n";

static const char *display_frag_src =
  "precision mediump float;\n"
  "uniform sampler2D u_texture;\n"
  "varying vec2 v_texCoord;\n"
  "void main() {\n"
  "  gl_FragColor = texture2D(u_texture, v_texCoord);\n"
  "}\n";

void init_shaders(); // Forward declaration for init_shaders

static void update_offsets() {
  for (int i = 0; i < 2; ++i) {
    yOffset[i] += ySpeed[i] * direction[i];
    if (yOffset[i] > 0.5f || yOffset[i] < -0.5f)
      direction[i] = -direction[i];
  }
}

static void create_fbo_and_tex(int idx) {
  int w = (idx < 2 ? screen_width/2 : screen_width);
  int h = screen_height;
  glGenTextures(1, &tex[idx]);
  glBindTexture(GL_TEXTURE_2D, tex[idx]);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glGenFramebuffers(1, &fbo[idx]);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo[idx]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex[idx], 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    fprintf(stderr, "FBO %d not complete!\n", idx);
}

int init_triangle() {
  // compile tri shader
  GLuint vs = compile_shader(GL_VERTEX_SHADER,   vert_src);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, frag_src);
  triangle_program = glCreateProgram();
  glAttachShader(triangle_program, vs);
  glAttachShader(triangle_program, fs);
  link_program(triangle_program);
  glDeleteShader(vs); glDeleteShader(fs);

  // make two identical VBOs
  float tri_verts[9] = {
     0.0f,  0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f
  };
  glGenBuffers(2, triangle_vbo);
  for (int i = 0; i < 2; ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo[i]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tri_verts), tri_verts, GL_STATIC_DRAW);
  }

  // create 3 FBOs+textures:
  create_fbo_and_tex(0);
  create_fbo_and_tex(1);
  create_fbo_and_tex(2);

  init_shaders();
  
  return 0;
}

void init_shaders() {
  // display
  GLuint dvs = compile_shader(GL_VERTEX_SHADER,   display_vert_src);
  GLuint dfs = compile_shader(GL_FRAGMENT_SHADER, display_frag_src);
  display_program = glCreateProgram();
  glAttachShader(display_program, dvs);
  glAttachShader(display_program, dfs);
  link_program(display_program);
  glDeleteShader(dvs); glDeleteShader(dfs);
}

static void draw_fullscreen_quad(GLuint prog) {
  // uv quad from (0,0) to (1,1)
  static const float quad_uvs[] = {
    0.f,0.f,  1.f,0.f,  0.f,1.f,  1.f,1.f
  };
  GLuint vbo; 
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_uvs), quad_uvs, GL_STATIC_DRAW);

  GLint a_uv = glGetAttribLocation(prog, "a_texCoord");
  glEnableVertexAttribArray(a_uv);
  glVertexAttribPointer(a_uv, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(a_uv);
  glDeleteBuffers(1, &vbo);
}
void draw_triangle() {
  update_offsets();

  // 1) render tri0 → tex[0], tri1 → tex[1]
  glUseProgram(triangle_program);
  GLint off_loc = glGetUniformLocation(triangle_program, "u_y_offset");
  GLint pos_loc = glGetAttribLocation(triangle_program, "a_vertex");
  for (int i = 0; i < 2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
    // full‑size of each half‑texture is half width, full height:
    glViewport(0, 0, screen_width/2, screen_height);
    glClearColor((0.5*i), 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1f(off_loc, yOffset[i]);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo[i]);
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(pos_loc);
  }

  // 2) composite tex[0] on left, tex[1] on right -> tex[2]
  glBindFramebuffer(GL_FRAMEBUFFER, fbo[2]);
  // clear entire combined texture
  glViewport(0, 0, screen_width, screen_height);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  // use the simple display shader (it just samples u_texture)
  glUseProgram(display_program);
  GLint tex_loc = glGetUniformLocation(display_program, "u_texture");

  // draw left half
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex[0]);
  glUniform1i(tex_loc, 0);
  glViewport(0, 0, screen_width/2, screen_height);
  draw_fullscreen_quad(display_program);

  // draw right half
  glBindTexture(GL_TEXTURE_2D, tex[1]);
  glUniform1i(tex_loc, 0);
  glViewport(screen_width/2, 0, screen_width/2, screen_height);
  draw_fullscreen_quad(display_program);

  // 3) display tex[2] to the default framebuffer (screen)
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, screen_width, screen_height);
  glClearColor(1.0, 0.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(display_program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex[2]);
  glUniform1i(tex_loc, 0);
  draw_fullscreen_quad(display_program);
}
