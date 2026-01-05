#include "render/UI.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>

#include "core/Application.h"
#include "game/Chat.h"
#include "game/Console.h"
#include "game/Game.h"
#include "render/Renderer.h"

void RenderConsole(const Application& app, const Console& console) {
    if (!console.open) {
        return;
    }
    
    const int max_draw = 10;
    int line_count = static_cast<int>(console.lines.size());
    int start = (line_count > max_draw) ? (line_count - max_draw) : 0;
    int draw_count = line_count - start;
    
    const float font_scale = 2.5f;
    const float line_height = 8.0f * font_scale;
    const float pad = 10.0f;
    float box_h = (draw_count + 1) * line_height + pad * 2.0f;
    float box_w = 620.0f;
    float box_x = 16.0f;
    float box_y = app.window_height - box_h - 16.0f;
    
    DrawRect2D(box_x, box_y, box_w, box_h, 0.05f, 0.05f, 0.08f, 0.75f);
    
    float text_x = box_x + pad;
    float y = box_y + box_h - pad - line_height;
    for (int i = start; i < line_count; ++i) {
        DrawText2D(console.lines[i].c_str(), text_x, y, font_scale, 0.9f, 0.9f, 0.95f, 0.95f);
        y -= line_height;
    }
    
    std::string input_line = "> " + console.input + "_";
    DrawText2D(input_line.c_str(), text_x, box_y + pad, font_scale, 0.95f, 0.85f, 0.9f, 0.95f);
}

void RenderChat(const Application& app, const Chat& chat, double time_sec) {
    if (!chat.open && chat.messages.empty()) {
        return;
    }
    
    const float font_scale = 2.5f;
    const float line_height = 8.0f * font_scale;
    const float pad = 10.0f;
    const float max_box_h = static_cast<float>(app.window_height) * 0.35f;
    float box_w = 620.0f;
    float box_x = 16.0f;
    float box_y = 16.0f;
    
    int max_visible = static_cast<int>((max_box_h - pad * 2.0f - (chat.open ? line_height : 0.0f)) / line_height);
    if (max_visible < 1) {
        max_visible = 1;
    }
    
    int total = static_cast<int>(chat.messages.size());
    
    if (chat.open) {
        int max_scroll = total - max_visible;
        if (max_scroll < 0) {
            max_scroll = 0;
        }
        int start = total - max_visible - chat.scroll;
        if (start < 0) {
            start = 0;
        }
        int end = start + max_visible;
        if (end > total) {
            end = total;
        }
        int visible = end - start;
        float box_h = pad * 2.0f + (visible + 1) * line_height;
        if (box_h > max_box_h) {
            box_h = max_box_h;
        }
        DrawRect2D(box_x, box_y, box_w, box_h, 0.05f, 0.05f, 0.08f, 0.7f);
        
        float text_x = box_x + pad;
        float base_y = box_y + pad + line_height;
        float y = base_y + line_height * static_cast<float>(visible - 1);
        for (int i = start; i < end; ++i) {
            DrawText2D(chat.messages[i].text.c_str(), text_x, y, font_scale, 0.9f, 0.95f, 0.9f, 0.95f);
            y -= line_height;
        }
        std::string input_line = chat.input + "_";
        DrawText2D(input_line.c_str(), text_x, box_y + pad, font_scale, 0.95f, 0.95f, 0.9f, 0.95f);
    } else {
        int shown = 0;
        float max_alpha = 0.0f;
        struct VisibleChat {
            int index;
            float alpha;
        };
        std::vector<VisibleChat> visible;
        visible.reserve(static_cast<size_t>(max_visible));
        
        for (int i = total - 1; i >= 0; --i) {
            double age = time_sec - chat.messages[i].time;
            if (age > chat.fade_end) {
                continue;
            }
            float alpha = ChatAlpha(age, chat.fade_start, chat.fade_end);
            if (alpha <= 0.0f) {
                continue;
            }
            visible.push_back({i, alpha});
            if (alpha > max_alpha) {
                max_alpha = alpha;
            }
            ++shown;
            if (shown >= max_visible) {
                break;
            }
        }
        
        if (!visible.empty()) {
            float box_h = pad * 2.0f + static_cast<float>(visible.size()) * line_height;
            if (box_h > max_box_h) {
                box_h = max_box_h;
            }
            DrawRect2D(box_x, box_y, box_w, box_h, 0.05f, 0.05f, 0.08f, 0.6f * max_alpha);
            
            float text_x = box_x + pad;
            float base_y = box_y + pad;
            float y = base_y + line_height * static_cast<float>(visible.size() - 1);
            for (auto it = visible.rbegin(); it != visible.rend(); ++it) {
                const auto& msg = chat.messages[it->index];
                DrawText2D(msg.text.c_str(), text_x, y, font_scale, 0.9f, 0.95f, 0.9f, it->alpha);
                y -= line_height;
            }
        }
    }
}

void RenderFPS(const Application& app, float x, float y, float scale) {
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "FPS: %.1f (%.2f ms)", app.fps, app.frame_time);
    DrawText2D(buffer, x, y, scale, 0.9f, 0.9f, 0.9f, 0.8f);
}

void RenderUI(const Application& app, const Game& game) {
    BeginUI(app.window_width, app.window_height);
    
    RenderConsole(app, game.console);
    RenderChat(app, game.chat, game.time_sec);
    
    if (game.show_fps) {
        RenderFPS(app, 16.0f, app.window_height - 32.0f, 1.5f);
    }
    
    EndUI();
    DrawCrosshair(app.window_width, app.window_height, 8, 0.9f, 0.9f, 0.9f);
}

