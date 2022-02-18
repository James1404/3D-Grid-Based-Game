#include "window.h"

void window_t::init(std::string title, int size_x, int size_y)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		window_size_x = size_x;
		window_size_y = size_y;

		window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_size_x, window_size_y, SDL_WINDOW_OPENGL);		

		window_is_running = true;
	}
	else
	{
		window_is_running = false;
	}
}

void window_t::shutdown()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

bool window_t::handle_events()
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
