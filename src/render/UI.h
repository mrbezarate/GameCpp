#pragma once

#include "game/Chat.h"
#include "game/Console.h"

struct Application;
struct Game;

void RenderUI(const Application& app, const Game& game);
void RenderConsole(const Application& app, const Console& console);
void RenderChat(const Application& app, const Chat& chat, double time_sec);
void RenderFPS(const Application& app, float x, float y, float scale);


