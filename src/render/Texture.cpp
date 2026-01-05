#include "render/Texture.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <cstdio>
#include <vector>

#ifndef APIENTRY
#if defined(_WIN32)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

#ifndef GL_TEXTURE_LOD_BIAS
#define GL_TEXTURE_LOD_BIAS 0x8501
#endif

typedef void(APIENTRY* PFNGLGENERATEMIPMAPPROC)(GLenum);
static PFNGLGENERATEMIPMAPPROC glGenerateMipmap_ = nullptr;

static bool LoadGenerateMipmap() {
    if (glGenerateMipmap_) {
        return true;
    }
    glGenerateMipmap_ = reinterpret_cast<PFNGLGENERATEMIPMAPPROC>(SDL_GL_GetProcAddress("glGenerateMipmap"));
    if (!glGenerateMipmap_) {
        glGenerateMipmap_ = reinterpret_cast<PFNGLGENERATEMIPMAPPROC>(SDL_GL_GetProcAddress("glGenerateMipmapEXT"));
    }
    return glGenerateMipmap_ != nullptr;
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static Texture2D CreateTextureFromRGBA(const unsigned char* data, int width, int height) {
    Texture2D tex{};
    tex.width = width;
    tex.height = height;
    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    if (LoadGenerateMipmap()) {
        glGenerateMipmap_(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.6f);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

Texture2D LoadTexture2D(const char* path) {
    Texture2D tex{};
    stbi_set_flip_vertically_on_load(1);
    int width = 0;
    int height = 0;
    int comp = 0;
    unsigned char* data = stbi_load(path, &width, &height, &comp, 4);
    if (!data) {
        std::printf("Failed to load texture: %s\n", path);
        return tex;
    }
    tex = CreateTextureFromRGBA(data, width, height);
    stbi_image_free(data);
    return tex;
}

Texture2D CreateCheckerTexture(int size, int check_size) {
    if (size <= 0 || check_size <= 0) {
        return Texture2D{};
    }
    std::vector<unsigned char> pixels(static_cast<size_t>(size) * size * 4);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            int cell = ((x / check_size) ^ (y / check_size)) & 1;
            unsigned char r = cell ? 240 : 150;
            unsigned char g = cell ? 240 : 70;
            unsigned char b = cell ? 240 : 200;
            size_t idx = static_cast<size_t>((y * size + x) * 4);
            pixels[idx + 0] = r;
            pixels[idx + 1] = g;
            pixels[idx + 2] = b;
            pixels[idx + 3] = 255;
        }
    }
    return CreateTextureFromRGBA(pixels.data(), size, size);
}

Texture2D CreateRoadTexture(int width, int height, int stripe_width, int dash_length, int dash_gap) {
    if (width <= 0 || height <= 0 || stripe_width <= 0) {
        return Texture2D{};
    }
    if (dash_length <= 0) {
        dash_length = 16;
    }
    if (dash_gap < 0) {
        dash_gap = 8;
    }
    std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * 4);
    int center = width / 2;
    for (int y = 0; y < height; ++y) {
        bool dash_on = ((y / (dash_length + dash_gap)) % 2) == 0;
        for (int x = 0; x < width; ++x) {
            unsigned char r = 18;
            unsigned char g = 18;
            unsigned char b = 18;
            int dx = std::abs(x - center);
            if (dx <= stripe_width && dash_on) {
                r = 230;
                g = 230;
                b = 230;
            }
            size_t idx = static_cast<size_t>((y * width + x) * 4);
            pixels[idx + 0] = r;
            pixels[idx + 1] = g;
            pixels[idx + 2] = b;
            pixels[idx + 3] = 255;
        }
    }
    return CreateTextureFromRGBA(pixels.data(), width, height);
}

void DestroyTexture(Texture2D& tex) {
    if (tex.id != 0) {
        glDeleteTextures(1, &tex.id);
        tex.id = 0;
    }
    tex.width = 0;
    tex.height = 0;
}
