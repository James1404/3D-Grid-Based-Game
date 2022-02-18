#pragma once
#include <SDL.h>
#include <string>
#include <cassert>

struct window_t
{
	SDL_Window* window;
	SDL_Event window_event;
	bool window_is_running;

	int window_size_x, window_size_y;

	bool handle_events();

	void init(std::string title, int size_x, int size_y);
	void shutdown();

	static window_t& get()
	{
		static window_t* instance = NULL;
		if(instance == NULL)
		{
			instance = new window_t;
		}
		assert(instance);
		return *instance;
	}
};
