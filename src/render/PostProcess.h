#pragma once

#include <SDL_opengl.h>

struct PostProcess {
    GLuint fbo = 0;
    GLuint color_tex = 0;
    GLuint depth_tex = 0;
    GLuint program = 0;
    GLuint vs = 0;
    GLuint fs = 0;
    int width = 0;
    int height = 0;
    GLint u_tex = -1;
    GLint u_depth = -1;
    GLint u_time = -1;
    GLint u_resolution = -1;
    GLint u_near_far = -1;
    bool valid = false;
};

bool InitPostProcess(PostProcess& pp, int width, int height);
void ResizePostProcess(PostProcess& pp, int width, int height);
void BeginPostProcess(const PostProcess& pp);
void EndPostProcess(const PostProcess& pp, float time_sec, float near_plane, float far_plane);
void DestroyPostProcess(PostProcess& pp);
