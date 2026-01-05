#include "render/Renderer.h"

#include <SDL_opengl.h>
#include <cmath>
#include <cstring>

#include "render/Texture.h"

static void SetPerspective(float fov_deg, float aspect, float znear, float zfar) {
    float fov_rad = fov_deg * 0.017453292f;
    float f = std::tan(fov_rad * 0.5f);
    float top = znear * f;
    float bottom = -top;
    float right = top * aspect;
    float left = -right;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(left, right, bottom, top, znear, zfar);
}

void Setup3D(int ww, int wh, float fov_deg, float cam_yaw_deg, float cam_pitch_deg, Vec3 cam_pos) {
    float aspect = (wh > 0) ? static_cast<float>(ww) / static_cast<float>(wh) : 16.0f / 9.0f;
    glViewport(0, 0, ww, wh);
    SetPerspective(fov_deg, aspect, 0.1f, 2000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-cam_pitch_deg, 1.0f, 0.0f, 0.0f);
    glRotatef(-cam_yaw_deg, 0.0f, 1.0f, 0.0f);
    glTranslatef(-cam_pos.x, -cam_pos.y, -cam_pos.z);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void DrawBox(const AABB& box, float r, float g, float b) {
    float x0 = box.center.x - box.half.x;
    float x1 = box.center.x + box.half.x;
    float y0 = box.center.y - box.half.y;
    float y1 = box.center.y + box.half.y;
    float z0 = box.center.z - box.half.z;
    float z1 = box.center.z + box.half.z;
    glDisable(GL_TEXTURE_2D);
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(x0, y0, z1); glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
    glVertex3f(x1, y0, z0); glVertex3f(x0, y0, z0); glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0);
    glVertex3f(x0, y0, z0); glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1); glVertex3f(x0, y1, z0);
    glVertex3f(x1, y0, z1); glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0); glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z1); glVertex3f(x1, y1, z1); glVertex3f(x1, y1, z0); glVertex3f(x0, y1, z0);
    glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1); glVertex3f(x0, y0, z1);
    glEnd();
}

void DrawBoxTextured(const AABB& box, const Texture2D& texture, float tint_r, float tint_g, float tint_b, float tile_size) {
    if (texture.id == 0) {
        DrawBox(box, tint_r, tint_g, tint_b);
        return;
    }
    if (tile_size <= 0.0f) {
        tile_size = 1.0f;
    }
    float x0 = box.center.x - box.half.x;
    float x1 = box.center.x + box.half.x;
    float y0 = box.center.y - box.half.y;
    float y1 = box.center.y + box.half.y;
    float z0 = box.center.z - box.half.z;
    float z1 = box.center.z + box.half.z;
    float size_x = (box.half.x * 2.0f) / tile_size;
    float size_y = (box.half.y * 2.0f) / tile_size;
    float size_z = (box.half.z * 2.0f) / tile_size;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glColor3f(tint_r, tint_g, tint_b);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);    glVertex3f(x0, y0, z1);
    glTexCoord2f(size_x, 0.0f);  glVertex3f(x1, y0, z1);
    glTexCoord2f(size_x, size_y); glVertex3f(x1, y1, z1);
    glTexCoord2f(0.0f, size_y);  glVertex3f(x0, y1, z1);

    glTexCoord2f(0.0f, 0.0f);    glVertex3f(x1, y0, z0);
    glTexCoord2f(size_x, 0.0f);  glVertex3f(x0, y0, z0);
    glTexCoord2f(size_x, size_y); glVertex3f(x0, y1, z0);
    glTexCoord2f(0.0f, size_y);  glVertex3f(x1, y1, z0);

    glTexCoord2f(0.0f, 0.0f);    glVertex3f(x0, y0, z0);
    glTexCoord2f(size_z, 0.0f);  glVertex3f(x0, y0, z1);
    glTexCoord2f(size_z, size_y); glVertex3f(x0, y1, z1);
    glTexCoord2f(0.0f, size_y);  glVertex3f(x0, y1, z0);

    glTexCoord2f(0.0f, 0.0f);    glVertex3f(x1, y0, z1);
    glTexCoord2f(size_z, 0.0f);  glVertex3f(x1, y0, z0);
    glTexCoord2f(size_z, size_y); glVertex3f(x1, y1, z0);
    glTexCoord2f(0.0f, size_y);  glVertex3f(x1, y1, z1);

    glTexCoord2f(0.0f, 0.0f);    glVertex3f(x0, y1, z1);
    glTexCoord2f(size_x, 0.0f);  glVertex3f(x1, y1, z1);
    glTexCoord2f(size_x, size_z); glVertex3f(x1, y1, z0);
    glTexCoord2f(0.0f, size_z);  glVertex3f(x0, y1, z0);

    glTexCoord2f(0.0f, 0.0f);    glVertex3f(x0, y0, z0);
    glTexCoord2f(size_x, 0.0f);  glVertex3f(x1, y0, z0);
    glTexCoord2f(size_x, size_z); glVertex3f(x1, y0, z1);
    glTexCoord2f(0.0f, size_z);  glVertex3f(x0, y0, z1);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

static void DrawDigitSegments(float x, float y, float z, float w, float h, float t, int digit) {
    static const int segs[10] = {
        0x3F, // 0
        0x06, // 1
        0x5B, // 2
        0x4F, // 3
        0x66, // 4
        0x6D, // 5
        0x7D, // 6
        0x07, // 7
        0x7F, // 8
        0x6F  // 9
    };
    int mask = (digit >= 0 && digit <= 9) ? segs[digit] : 0;
    auto quad = [z](float x0, float y0, float x1, float y1) {
        glVertex3f(x0, y0, z);
        glVertex3f(x1, y0, z);
        glVertex3f(x1, y1, z);
        glVertex3f(x0, y1, z);
    };

    float x0 = x;
    float x1 = x + w;
    float y0 = y;
    float y1 = y + h;
    float xm0 = x + t;
    float xm1 = x + w - t;
    float ym0 = y + t;
    float ym1 = y + h - t;
    float ymid0 = y + h * 0.5f - t * 0.5f;
    float ymid1 = ymid0 + t;

    if (mask & 0x01) { // top
        quad(xm0, ym1, xm1, y1);
    }
    if (mask & 0x02) { // upper right
        quad(xm1, ymid1, x1, ym1);
    }
    if (mask & 0x04) { // lower right
        quad(xm1, ym0, x1, ymid0);
    }
    if (mask & 0x08) { // bottom
        quad(xm0, y0, xm1, ym0);
    }
    if (mask & 0x10) { // lower left
        quad(x0, ym0, xm0, ymid0);
    }
    if (mask & 0x20) { // upper left
        quad(x0, ymid1, xm0, ym1);
    }
    if (mask & 0x40) { // middle
        quad(xm0, ymid0, xm1, ymid1);
    }
}

void DrawBuildingNumber(const AABB& box, int number, Facing facing) {
    if (number <= 0) {
        return;
    }
    int digits[3] = {0, 0, 0};
    int count = 0;
    int n = number;
    while (n > 0 && count < 3) {
        digits[count++] = n % 10;
        n /= 10;
    }
    if (count == 0) {
        return;
    }

    float face_width = (facing == Facing::PosZ || facing == Facing::NegZ) ? box.half.x : box.half.z;
    float w = face_width * 0.6f;
    if (w < 1.2f) {
        w = 1.2f;
    } else if (w > 3.6f) {
        w = 3.6f;
    }
    float h = w * 1.6f;
    float t = w * 0.18f;
    float gap = w * 0.2f;
    float total_w = count * w + (count - 1) * gap;
    float start_x = -total_w * 0.5f;
    float y = box.center.y + box.half.y - h - 0.5f;
    float min_y = box.center.y - box.half.y + 0.3f;
    if (y < min_y) {
        y = box.center.y - h * 0.5f;
    }

    float origin_x = box.center.x;
    float origin_z = box.center.z;
    float yaw = 0.0f;
    const float offset = 0.06f;
    switch (facing) {
        case Facing::PosZ:
            origin_z = box.center.z + box.half.z + offset;
            yaw = 0.0f;
            break;
        case Facing::NegZ:
            origin_z = box.center.z - box.half.z - offset;
            yaw = 180.0f;
            break;
        case Facing::PosX:
            origin_x = box.center.x + box.half.x + offset;
            yaw = -90.0f;
            break;
        case Facing::NegX:
            origin_x = box.center.x - box.half.x - offset;
            yaw = 90.0f;
            break;
    }

    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(origin_x, y, origin_z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    for (int i = 0; i < count; ++i) {
        int digit = digits[count - 1 - i];
        float x = start_x + i * (w + gap);
        DrawDigitSegments(x, 0.0f, 0.0f, w, h, t, digit);
    }
    glEnd();
    glPopMatrix();
}

void BeginUI(int ww, int wh) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(ww), 0.0, static_cast<double>(wh), -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void EndUI() {
    glDisable(GL_BLEND);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void DrawRect2D(float x, float y, float w, float h, float r, float g, float b, float a) {
    glDisable(GL_TEXTURE_2D);
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

struct Glyph {
    unsigned char rows[7];
};

static Glyph GetGlyph(char c) {
    if (c >= 'a' && c <= 'z') {
        c = static_cast<char>(c - 'a' + 'A');
    }
    switch (c) {
        case '0': return {{0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}};
        case '1': return {{0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}};
        case '2': return {{0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}};
        case '3': return {{0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E}};
        case '4': return {{0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}};
        case '5': return {{0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E}};
        case '6': return {{0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E}};
        case '7': return {{0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}};
        case '8': return {{0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}};
        case '9': return {{0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E}};
        case 'A': return {{0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}};
        case 'B': return {{0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}};
        case 'C': return {{0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}};
        case 'D': return {{0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}};
        case 'E': return {{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}};
        case 'F': return {{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}};
        case 'G': return {{0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E}};
        case 'H': return {{0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}};
        case 'I': return {{0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}};
        case 'J': return {{0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}};
        case 'K': return {{0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}};
        case 'L': return {{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}};
        case 'M': return {{0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11}};
        case 'N': return {{0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}};
        case 'O': return {{0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}};
        case 'P': return {{0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}};
        case 'Q': return {{0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}};
        case 'R': return {{0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}};
        case 'S': return {{0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}};
        case 'T': return {{0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}};
        case 'U': return {{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}};
        case 'V': return {{0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}};
        case 'W': return {{0x11, 0x11, 0x11, 0x11, 0x15, 0x1B, 0x11}};
        case 'X': return {{0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}};
        case 'Y': return {{0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}};
        case 'Z': return {{0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}};
        case ' ': return {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
        case '.': return {{0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06}};
        case ',': return {{0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x04}};
        case ':': return {{0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00}};
        case ';': return {{0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x04}};
        case '!': return {{0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}};
        case '?': return {{0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04}};
        case '-': return {{0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00}};
        case '_': return {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}};
        case '+': return {{0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00}};
        case '=': return {{0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00}};
        case '/': return {{0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00}};
        case '\\': return {{0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00}};
        case '\'': return {{0x06, 0x06, 0x04, 0x00, 0x00, 0x00, 0x00}};
        case '"': return {{0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00}};
        case '(': return {{0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}};
        case ')': return {{0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08}};
        case '[': return {{0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E}};
        case ']': return {{0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E}};
        default:
            return {{0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F}};
    }
}

void DrawText2D(const char* text, float x, float y, float scale, float r, float g, float b, float a) {
    if (!text || std::strlen(text) == 0) {
        return;
    }
    if (scale <= 0.1f) {
        scale = 1.0f;
    }
    const float px = scale;
    const float py = scale;
    float cursor_x = x;
    glDisable(GL_TEXTURE_2D);
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    const unsigned char* ptr = reinterpret_cast<const unsigned char*>(text);
    while (*ptr) {
        unsigned char ch = *ptr;
        int advance = 1;
        if (ch >= 0x80) {
            if ((ch & 0xE0) == 0xC0) {
                advance = 2;
            } else if ((ch & 0xF0) == 0xE0) {
                advance = 3;
            } else if ((ch & 0xF8) == 0xF0) {
                advance = 4;
            }
            ch = '?';
        }
        ptr += advance;
        if (ch == '\n') {
            cursor_x = x;
            y -= 8.0f * scale;
            continue;
        }
        Glyph glyph = GetGlyph(static_cast<char>(ch));
        for (int row = 0; row < 7; ++row) {
            unsigned char bits = glyph.rows[row];
            if (!bits) {
                continue;
            }
            for (int col = 0; col < 5; ++col) {
                if (bits & (1u << (4 - col))) {
                    float x0 = cursor_x + col * px;
                    float y0 = y + (6 - row) * py;
                    glVertex2f(x0, y0);
                    glVertex2f(x0 + px, y0);
                    glVertex2f(x0 + px, y0 + py);
                    glVertex2f(x0, y0 + py);
                }
            }
        }
        cursor_x += 6.0f * scale;
    }
    glEnd();
}

void DrawCrosshair(int ww, int wh, int size, float r, float g, float b) {
    float cx = ww * 0.5f;
    float cy = wh * 0.5f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(ww), 0.0, static_cast<double>(wh), -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);

    glColor3f(r, g, b);
    glBegin(GL_LINES);
    glVertex2f(cx - size, cy);
    glVertex2f(cx + size, cy);
    glVertex2f(cx, cy - size);
    glVertex2f(cx, cy + size);
    glEnd();
}
