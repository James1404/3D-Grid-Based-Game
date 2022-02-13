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

void init_input();
void shutdown_input();
void update_input();

bool input_button_down(std::string button);
bool input_button_pressed(std::string button);
bool input_button_released(std::string button);

bool input_key_down(SDL_Scancode key);
bool input_key_pressed(SDL_Scancode key);
bool input_key_released(SDL_Scancode key);

bool input_mouse_button_down(mouse_button button);
bool input_mouse_button_pressed(mouse_button button);
bool input_mouse_button_released(mouse_button button);

const glm::ivec2 input_get_mouse_pos();
const glm::ivec2 input_get_previous_mouse_pos();
const glm::ivec2 input_get_mouse_delta();

void save_input();
void load_input();
