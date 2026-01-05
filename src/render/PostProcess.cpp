#include "render/PostProcess.h"

#include <SDL.h>

#include <cstdio>
#include <cstring>
#include <string>

#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif
#ifndef GL_RENDERBUFFER
#define GL_RENDERBUFFER 0x8D41
#endif
#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0 0x8CE0
#endif
#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT 0x8D00
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#endif
#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24 0x81A6
#endif
#ifndef GL_TEXTURE_COMPARE_MODE
#define GL_TEXTURE_COMPARE_MODE 0x884C
#endif
#ifndef GL_COMPARE_REF_TO_TEXTURE
#define GL_COMPARE_REF_TO_TEXTURE 0x884E
#endif
#ifndef APIENTRY
#if defined(_WIN32)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

typedef GLuint(APIENTRY* PFNGLCREATESHADERPROC)(GLenum);
typedef void(APIENTRY* PFNGLSHADERSOURCEPROC)(GLuint, GLsizei, const GLchar* const*, const GLint*);
typedef void(APIENTRY* PFNGLCOMPILESHADERPROC)(GLuint);
typedef void(APIENTRY* PFNGLGETSHADERIVPROC)(GLuint, GLenum, GLint*);
typedef void(APIENTRY* PFNGLGETSHADERINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef GLuint(APIENTRY* PFNGLCREATEPROGRAMPROC)(void);
typedef void(APIENTRY* PFNGLATTACHSHADERPROC)(GLuint, GLuint);
typedef void(APIENTRY* PFNGLLINKPROGRAMPROC)(GLuint);
typedef void(APIENTRY* PFNGLGETPROGRAMIVPROC)(GLuint, GLenum, GLint*);
typedef void(APIENTRY* PFNGLGETPROGRAMINFOLOGPROC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void(APIENTRY* PFNGLUSEPROGRAMPROC)(GLuint);
typedef void(APIENTRY* PFNGLDELETESHADERPROC)(GLuint);
typedef void(APIENTRY* PFNGLDELETEPROGRAMPROC)(GLuint);
typedef GLint(APIENTRY* PFNGLGETUNIFORMLOCATIONPROC)(GLuint, const GLchar*);
typedef void(APIENTRY* PFNGLUNIFORM1IPROC)(GLint, GLint);
typedef void(APIENTRY* PFNGLUNIFORM1FPROC)(GLint, GLfloat);
typedef void(APIENTRY* PFNGLUNIFORM2FPROC)(GLint, GLfloat, GLfloat);
typedef void(APIENTRY* PFNGLACTIVETEXTUREPROC)(GLenum);

typedef void(APIENTRY* PFNGLGENFRAMEBUFFERSPROC)(GLsizei, GLuint*);
typedef void(APIENTRY* PFNGLBINDFRAMEBUFFERPROC)(GLenum, GLuint);
typedef void(APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLenum(APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum);
typedef void(APIENTRY* PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei, const GLuint*);

static PFNGLCREATESHADERPROC glCreateShader_ = nullptr;
static PFNGLSHADERSOURCEPROC glShaderSource_ = nullptr;
static PFNGLCOMPILESHADERPROC glCompileShader_ = nullptr;
static PFNGLGETSHADERIVPROC glGetShaderiv_ = nullptr;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ = nullptr;
static PFNGLCREATEPROGRAMPROC glCreateProgram_ = nullptr;
static PFNGLATTACHSHADERPROC glAttachShader_ = nullptr;
static PFNGLLINKPROGRAMPROC glLinkProgram_ = nullptr;
static PFNGLGETPROGRAMIVPROC glGetProgramiv_ = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ = nullptr;
static PFNGLUSEPROGRAMPROC glUseProgram_ = nullptr;
static PFNGLDELETESHADERPROC glDeleteShader_ = nullptr;
static PFNGLDELETEPROGRAMPROC glDeleteProgram_ = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ = nullptr;
static PFNGLUNIFORM1IPROC glUniform1i_ = nullptr;
static PFNGLUNIFORM1FPROC glUniform1f_ = nullptr;
static PFNGLUNIFORM2FPROC glUniform2f_ = nullptr;
static PFNGLACTIVETEXTUREPROC glActiveTexture_ = nullptr;

static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers_ = nullptr;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer_ = nullptr;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D_ = nullptr;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus_ = nullptr;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers_ = nullptr;
static bool LoadGLFunctions() {
    static bool loaded = false;
    static bool ok = false;
    if (loaded) {
        return ok;
    }
    loaded = true;
    auto load = [](const char* name) -> void* {
        return SDL_GL_GetProcAddress(name);
    };

    glCreateShader_ = reinterpret_cast<PFNGLCREATESHADERPROC>(load("glCreateShader"));
    glShaderSource_ = reinterpret_cast<PFNGLSHADERSOURCEPROC>(load("glShaderSource"));
    glCompileShader_ = reinterpret_cast<PFNGLCOMPILESHADERPROC>(load("glCompileShader"));
    glGetShaderiv_ = reinterpret_cast<PFNGLGETSHADERIVPROC>(load("glGetShaderiv"));
    glGetShaderInfoLog_ = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(load("glGetShaderInfoLog"));
    glCreateProgram_ = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(load("glCreateProgram"));
    glAttachShader_ = reinterpret_cast<PFNGLATTACHSHADERPROC>(load("glAttachShader"));
    glLinkProgram_ = reinterpret_cast<PFNGLLINKPROGRAMPROC>(load("glLinkProgram"));
    glGetProgramiv_ = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(load("glGetProgramiv"));
    glGetProgramInfoLog_ = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(load("glGetProgramInfoLog"));
    glUseProgram_ = reinterpret_cast<PFNGLUSEPROGRAMPROC>(load("glUseProgram"));
    glDeleteShader_ = reinterpret_cast<PFNGLDELETESHADERPROC>(load("glDeleteShader"));
    glDeleteProgram_ = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(load("glDeleteProgram"));
    glGetUniformLocation_ = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(load("glGetUniformLocation"));
    glUniform1i_ = reinterpret_cast<PFNGLUNIFORM1IPROC>(load("glUniform1i"));
    glUniform1f_ = reinterpret_cast<PFNGLUNIFORM1FPROC>(load("glUniform1f"));
    glUniform2f_ = reinterpret_cast<PFNGLUNIFORM2FPROC>(load("glUniform2f"));
    glActiveTexture_ = reinterpret_cast<PFNGLACTIVETEXTUREPROC>(load("glActiveTexture"));

    glGenFramebuffers_ = reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(load("glGenFramebuffers"));
    glBindFramebuffer_ = reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(load("glBindFramebuffer"));
    glFramebufferTexture2D_ = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(load("glFramebufferTexture2D"));
    glCheckFramebufferStatus_ = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSPROC>(load("glCheckFramebufferStatus"));
    glDeleteFramebuffers_ = reinterpret_cast<PFNGLDELETEFRAMEBUFFERSPROC>(load("glDeleteFramebuffers"));
    if (!glGenFramebuffers_) {
        glGenFramebuffers_ = reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(load("glGenFramebuffersEXT"));
        glBindFramebuffer_ = reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(load("glBindFramebufferEXT"));
        glFramebufferTexture2D_ = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(load("glFramebufferTexture2DEXT"));
        glCheckFramebufferStatus_ = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSPROC>(load("glCheckFramebufferStatusEXT"));
        glDeleteFramebuffers_ = reinterpret_cast<PFNGLDELETEFRAMEBUFFERSPROC>(load("glDeleteFramebuffersEXT"));
    }

    ok = glCreateShader_ && glShaderSource_ && glCompileShader_ && glGetShaderiv_ &&
         glCreateProgram_ && glAttachShader_ && glLinkProgram_ && glGetProgramiv_ &&
         glUseProgram_ && glDeleteShader_ && glDeleteProgram_ && glGetUniformLocation_ &&
         glUniform1i_ && glUniform1f_ && glUniform2f_ &&
         glGenFramebuffers_ && glBindFramebuffer_ && glFramebufferTexture2D_ &&
         glCheckFramebufferStatus_ && glDeleteFramebuffers_;

    if (!ok) {
        std::printf("PostProcess: required GL functions not available.\n");
    }
    return ok;
}

static void LogShader(GLuint shader) {
    GLint length = 0;
    glGetShaderiv_(shader, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) {
        return;
    }
    std::string log;
    log.resize(static_cast<size_t>(length));
    glGetShaderInfoLog_(shader, length, nullptr, log.data());
    std::printf("Shader log:\n%s\n", log.c_str());
}

static void LogProgram(GLuint program) {
    GLint length = 0;
    glGetProgramiv_(program, GL_INFO_LOG_LENGTH, &length);
    if (length <= 1) {
        return;
    }
    std::string log;
    log.resize(static_cast<size_t>(length));
    glGetProgramInfoLog_(program, length, nullptr, log.data());
    std::printf("Program log:\n%s\n", log.c_str());
}

static GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader_(type);
    glShaderSource_(shader, 1, &src, nullptr);
    glCompileShader_(shader);
    GLint status = 0;
    glGetShaderiv_(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        LogShader(shader);
        glDeleteShader_(shader);
        return 0;
    }
    return shader;
}

static bool CreateProgram(PostProcess& pp) {
    const char* vs_src =
        "#version 120\n"
        "varying vec2 v_uv;\n"
        "void main(){\n"
        "  gl_Position = ftransform();\n"
        "  v_uv = gl_MultiTexCoord0.xy;\n"
        "}\n";
    const char* fs_src =
        "#version 120\n"
        "uniform sampler2D u_tex;\n"
        "uniform sampler2D u_depth;\n"
        "uniform vec2 u_resolution;\n"
        "uniform vec2 u_near_far;\n"
        "uniform float u_time;\n"
        "varying vec2 v_uv;\n"
        "float luma(vec3 c){return dot(c, vec3(0.299,0.587,0.114));}\n"
        "float noise(vec2 p){return fract(sin(dot(p, vec2(12.9898,78.233))) * 43758.5453);}\n"
        "float linear_depth(float z){\n"
        "  float n = u_near_far.x;\n"
        "  float f = u_near_far.y;\n"
        "  float ndc = z * 2.0 - 1.0;\n"
        "  return (2.0 * n * f) / (f + n - ndc * (f - n));\n"
        "}\n"
        "void main(){\n"
        "  vec2 px = 1.0 / u_resolution;\n"
        "  vec3 c = texture2D(u_tex, v_uv).rgb;\n"
        "  vec3 n = texture2D(u_tex, v_uv + vec2(0.0, px.y)).rgb;\n"
        "  vec3 s = texture2D(u_tex, v_uv - vec2(0.0, px.y)).rgb;\n"
        "  vec3 e = texture2D(u_tex, v_uv + vec2(px.x, 0.0)).rgb;\n"
        "  vec3 w = texture2D(u_tex, v_uv - vec2(px.x, 0.0)).rgb;\n"
        "  vec3 d1 = texture2D(u_tex, v_uv + px * vec2(1.5, 1.5)).rgb;\n"
        "  vec3 d2 = texture2D(u_tex, v_uv + px * vec2(-1.5, 1.5)).rgb;\n"
        "  vec3 d3 = texture2D(u_tex, v_uv + px * vec2(1.5, -1.5)).rgb;\n"
        "  vec3 d4 = texture2D(u_tex, v_uv + px * vec2(-1.5, -1.5)).rgb;\n"
        "  vec3 f1 = texture2D(u_tex, v_uv + px * vec2(3.0, 0.0)).rgb;\n"
        "  vec3 f2 = texture2D(u_tex, v_uv + px * vec2(-3.0, 0.0)).rgb;\n"
        "  vec3 f3 = texture2D(u_tex, v_uv + px * vec2(0.0, 3.0)).rgb;\n"
        "  vec3 f4 = texture2D(u_tex, v_uv + px * vec2(0.0, -3.0)).rgb;\n"
        "  float lc = luma(c);\n"
        "  float edge = max(max(abs(lc - luma(n)), abs(lc - luma(s))), max(abs(lc - luma(e)), abs(lc - luma(w))));\n"
        "  float aa = smoothstep(0.06, 0.22, edge);\n"
        "  vec3 blur5 = (c + n + s + e + w) * 0.2;\n"
        "  vec3 blur9 = (c + n + s + e + w + d1 + d2 + d3 + d4) / 9.0;\n"
        "  vec3 blur_far = (blur9 + (f1 + f2 + f3 + f4) * 0.25) * 0.5;\n"
        "  vec3 aa_color = mix(c, blur5, aa * 0.7);\n"
        "  float depth = linear_depth(texture2D(u_depth, v_uv).r);\n"
        "  float far_blur = smoothstep(5.0, 18.0, depth);\n"
        "  vec3 base = mix(aa_color, blur9, far_blur * 0.65);\n"
        "  float ground_mask = 1.0 - smoothstep(0.35, 0.7, v_uv.y);\n"
        "  float ground_blur = smoothstep(10.0, 35.0, depth) * ground_mask;\n"
        "  vec3 base2 = mix(base, blur_far, ground_blur * 0.75);\n"
        "  float b = smoothstep(0.65, 1.0, max(max(d1.r, d1.g), d1.b));\n"
        "  vec3 color = base2 + (d1 + d2 + d3 + d4) * 0.25 * (0.3 * b);\n"
        "  float g = dot(color, vec3(0.333));\n"
        "  color = mix(color, vec3(g), 0.12);\n"
        "  color *= vec3(0.96, 0.98, 1.05);\n"
        "  float vig = smoothstep(0.85, 0.2, length(v_uv - 0.5));\n"
        "  color *= mix(1.0, vig, 0.3);\n"
        "  float gr = noise(v_uv * u_resolution + u_time * 15.0) - 0.5;\n"
        "  color += gr * 0.02;\n"
        "  color = clamp(color, 0.0, 1.0);\n"
        "  gl_FragColor = vec4(color, 1.0);\n"
        "}\n";

    pp.vs = CompileShader(GL_VERTEX_SHADER, vs_src);
    pp.fs = CompileShader(GL_FRAGMENT_SHADER, fs_src);
    if (!pp.vs || !pp.fs) {
        return false;
    }

    pp.program = glCreateProgram_();
    glAttachShader_(pp.program, pp.vs);
    glAttachShader_(pp.program, pp.fs);
    glLinkProgram_(pp.program);
    GLint linked = 0;
    glGetProgramiv_(pp.program, GL_LINK_STATUS, &linked);
    if (!linked) {
        LogProgram(pp.program);
        glDeleteProgram_(pp.program);
        pp.program = 0;
        return false;
    }

    pp.u_tex = glGetUniformLocation_(pp.program, "u_tex");
    pp.u_depth = glGetUniformLocation_(pp.program, "u_depth");
    pp.u_time = glGetUniformLocation_(pp.program, "u_time");
    pp.u_resolution = glGetUniformLocation_(pp.program, "u_resolution");
    pp.u_near_far = glGetUniformLocation_(pp.program, "u_near_far");
    return true;
}

static bool CreateRenderTarget(PostProcess& pp, int width, int height) {
    if (pp.fbo != 0) {
        glDeleteFramebuffers_(1, &pp.fbo);
        pp.fbo = 0;
    }
    if (pp.color_tex != 0) {
        glDeleteTextures(1, &pp.color_tex);
        pp.color_tex = 0;
    }
    if (pp.depth_tex != 0) {
        glDeleteTextures(1, &pp.depth_tex);
        pp.depth_tex = 0;
    }

    pp.width = width;
    pp.height = height;

    glGenFramebuffers_(1, &pp.fbo);
    glBindFramebuffer_(GL_FRAMEBUFFER, pp.fbo);

    glGenTextures(1, &pp.color_tex);
    glBindTexture(GL_TEXTURE_2D, pp.color_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pp.color_tex, 0);

    glGenTextures(1, &pp.depth_tex);
    glBindTexture(GL_TEXTURE_2D, pp.depth_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, pp.depth_tex, 0);

    GLenum status = glCheckFramebufferStatus_(GL_FRAMEBUFFER);
    glBindFramebuffer_(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::printf("PostProcess: framebuffer incomplete.\n");
        return false;
    }
    return true;
}

bool InitPostProcess(PostProcess& pp, int width, int height) {
    DestroyPostProcess(pp);
    if (!LoadGLFunctions()) {
        return false;
    }
    if (!CreateProgram(pp)) {
        return false;
    }
    if (!CreateRenderTarget(pp, width, height)) {
        return false;
    }
    pp.valid = true;
    return true;
}

void ResizePostProcess(PostProcess& pp, int width, int height) {
    if (!pp.valid) {
        return;
    }
    if (width == pp.width && height == pp.height) {
        return;
    }
    CreateRenderTarget(pp, width, height);
}

void BeginPostProcess(const PostProcess& pp) {
    if (!pp.valid) {
        return;
    }
    glBindFramebuffer_(GL_FRAMEBUFFER, pp.fbo);
    glViewport(0, 0, pp.width, pp.height);
}

void EndPostProcess(const PostProcess& pp, float time_sec, float near_plane, float far_plane) {
    if (!pp.valid) {
        return;
    }

    glBindFramebuffer_(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, pp.width, pp.height);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glUseProgram_(pp.program);
    if (glActiveTexture_) {
        glActiveTexture_(GL_TEXTURE0);
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pp.color_tex);
    glUniform1i_(pp.u_tex, 0);
    if (glActiveTexture_) {
        glActiveTexture_(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, pp.depth_tex);
        glUniform1i_(pp.u_depth, 1);
        glActiveTexture_(GL_TEXTURE0);
    } else {
        glUniform1i_(pp.u_depth, 0);
    }
    glUniform1f_(pp.u_time, time_sec);
    glUniform2f_(pp.u_resolution, static_cast<float>(pp.width), static_cast<float>(pp.height));
    glUniform2f_(pp.u_near_far, near_plane, far_plane);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();

    if (glActiveTexture_) {
        glActiveTexture_(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        glActiveTexture_(GL_TEXTURE0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glUseProgram_(0);
}

void DestroyPostProcess(PostProcess& pp) {
    if (!LoadGLFunctions()) {
        std::memset(&pp, 0, sizeof(pp));
        return;
    }
    if (pp.program) {
        glDeleteProgram_(pp.program);
    }
    if (pp.vs) {
        glDeleteShader_(pp.vs);
    }
    if (pp.fs) {
        glDeleteShader_(pp.fs);
    }
    if (pp.color_tex) {
        glDeleteTextures(1, &pp.color_tex);
    }
    if (pp.depth_tex) {
        glDeleteTextures(1, &pp.depth_tex);
    }
    if (pp.fbo) {
        glDeleteFramebuffers_(1, &pp.fbo);
    }
    std::memset(&pp, 0, sizeof(pp));
}
