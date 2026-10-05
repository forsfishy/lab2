#pragma once

#include <SDL.h>

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;

void reset_scene();
void draw(SDL_Surface *surface);
void handle_key(SDL_Scancode key);
void set_pivot(int x, int y);
