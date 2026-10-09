#pragma once

#include <SDL.h>

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

void reset_scene();
void resize_scene(int width, int height);
void draw(SDL_Surface *surface);
void handle_key(SDL_Scancode key);
void set_pivot(int x, int y);
