#include "draw.h"

#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

SDL_Window *window;
SDL_Surface *surface;
bool quit = false;

void frame()
{
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT)
      quit = true;
    else if (e.type == SDL_KEYDOWN &&
             e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
      quit = true;
    else if (e.type == SDL_KEYDOWN)
      handle_key(e.key.keysym.scancode);
    else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
      set_pivot(e.button.x, e.button.y);
  }

  draw(surface);
  SDL_UpdateWindowSurface(window);
}

int main(int, char **)
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::printf("SDL_Init error: %s\n", SDL_GetError());
    return 1;
  }

  window =
      SDL_CreateWindow("Lab 2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    std::printf("SDL_CreateWindow error: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  surface = SDL_GetWindowSurface(window);
  reset_scene();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(frame, 0, 1);
#else
  while (!quit) {
    frame();
    SDL_Delay(16);
  }
  SDL_DestroyWindow(window);
  SDL_Quit();
#endif
  return 0;
}
