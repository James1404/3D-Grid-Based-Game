#include "window.h"

#include "renderer.h"

SDL_Window* window;
SDL_Event window_event;

bool window_is_running = false;

int window_size_x = 800, window_size_y = 600;

void init_window(std::string title, int size_x, int size_y)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		window_size_x = size_x;
		window_size_y = size_y;

		window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_size_x, window_size_y, SDL_WINDOW_OPENGL);		

		window_is_running = true;
	}
}

void shutdown_window()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

bool handle_window_events()
{
	if(!SDL_PollEvent(&window_event))
	{
		return false;
	}

	switch(window_event.type)
	{
	case SDL_QUIT:
		window_is_running = false;
		break;
	}

	return true;
}
