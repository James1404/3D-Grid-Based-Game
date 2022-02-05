#pragma once
#include <SDL.h>
#include <string>

extern SDL_Window* window;
extern SDL_Event window_event;
extern bool window_is_running;

extern int window_size_x, window_size_y;

void init_window(std::string title, int size_x, int size_y);
void shutdown_window();
bool handle_window_events();
