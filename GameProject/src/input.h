#pragma once
#include <string>

#include <glm.hpp>
#include <SDL.h>
#include <map>
#include <cassert>

enum class mouse_button {
	MOUSE_LEFT = 0,
	MOUSE_RIGHT,
	MOUSE_MIDDLE, 
	MOUSE_BACK,
	MOUSE_FORWARD
};

struct input_t
{
	const uint8_t* keyboard_state;
	uint8_t* previous_keyboard_state;
	int keyboard_state_size;

	Uint32 mouse_state;
	Uint32 previous_mouse_state;
	glm::ivec2 mouse_position;
	glm::ivec2 previous_mouse_position;

	std::multimap<std::string, unsigned int> mapped_inputs;

	void init();
	void shutdown();
	void update();

	bool button_down(std::string button);
	bool button_pressed(std::string button);
	bool button_released(std::string button);

	bool key_down(SDL_Scancode key);
	bool key_pressed(SDL_Scancode key);
	bool key_released(SDL_Scancode key);

	bool mouse_button_down(mouse_button button);
	bool mouse_button_pressed(mouse_button button);
	bool mouse_button_released(mouse_button button);

	const glm::ivec2 get_mouse_pos();
	const glm::ivec2 get_previous_mouse_pos();
	const glm::ivec2 get_mouse_delta();

	void save();
	void load();

	static input_t& get()
	{
		static input_t* instance = NULL;
		if(instance == NULL)
		{
			instance = new input_t;
		}
		assert(instance);
		return *instance;
	}
};
