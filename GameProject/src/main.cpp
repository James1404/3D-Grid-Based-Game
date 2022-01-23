#include <SDL.h>
#include "engine.h"

SDL_Event event;
bool update_sdl_events()
{
	if (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			return false;
			break;
			/*
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				renderer::screen_resolution_x = event.window.data1;
				renderer::screen_resolution_y = event.window.data2;
				SDL_SetWindowSize(renderer::window, renderer::screen_resolution_x, renderer::screen_resolution_y);
			}
			break;
			*/
		}
	}

	return true;
}

uint64_t NOW = SDL_GetPerformanceCounter(), LAST = 0;
double update_sdl_delta_time()
{
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();
	return (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());
}

int main(int argc, char* args[]) {

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {

	}

	engine_run(update_sdl_events, update_sdl_delta_time);

	return 0;
}
