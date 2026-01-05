#pragma once

#include <SDL_opengl.h>

struct Texture2D {
    GLuint id = 0;
    int width = 0;
    int height = 0;
};

Texture2D LoadTexture2D(const char* path);
Texture2D CreateCheckerTexture(int size, int check_size);
Texture2D CreateRoadTexture(int width, int height, int stripe_width, int dash_length, int dash_gap);
void DestroyTexture(Texture2D& tex);
