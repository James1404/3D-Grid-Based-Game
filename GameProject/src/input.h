#pragma once
#include <string>

#include <glm.hpp>
#include <SDL.h>
#include <map>

enum class mouse_button {
	MOUSE_LEFT = 0,
	MOUSE_RIGHT,
	MOUSE_MIDDLE, 
	MOUSE_BACK,
	MOUSE_FORWARD
};

struct input_manager_t {
	input_manager_t();
	~input_manager_t();

	const uint8_t* keyboard_state;
	uint8_t* previous_keyboard_state;
	int keyboard_state_size;

	Uint32 mouse_state;
	Uint32 previous_mouse_state;
	glm::ivec2 mouse_position;
	glm::ivec2 previous_mouse_position;

	std::multimap<std::string, unsigned int> mapped_inputs;

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

	// TODO: implement relative mouse positions.
	const glm::ivec2 get_relative_mouse_pos();
	const glm::ivec2 get_relative_previous_mouse_pos();
	const glm::ivec2 get_relative_mouse_delta();

	void save();
	void load();
};
