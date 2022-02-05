#include "input.h"

#include <fstream>
#include <sstream>
#include <memory>

#include "window.h"
#include "renderer.h"
#include "log.h"

static const uint8_t* keyboard_state;
static uint8_t* previous_keyboard_state;
static int keyboard_state_size;

static Uint32 mouse_state;
static Uint32 previous_mouse_state;
static glm::ivec2 mouse_position;
static glm::ivec2 previous_mouse_position;

static std::multimap<std::string, unsigned int> mapped_inputs;

void init_input()
{
	keyboard_state = SDL_GetKeyboardState(&keyboard_state_size);
	previous_keyboard_state = new Uint8[keyboard_state_size];
	memcpy(previous_keyboard_state, keyboard_state, keyboard_state_size);

	log_info("INITIALIZED INPUT");
}

void shutdown_input()
{
	delete[] previous_keyboard_state;
	previous_keyboard_state = NULL;
}

void update_input()
{
	previous_mouse_state = mouse_state;
	previous_mouse_position = mouse_position;
	mouse_state = SDL_GetMouseState(&mouse_position.x, &mouse_position.y);
	memcpy(previous_keyboard_state, keyboard_state, keyboard_state_size);
}

bool input_button_down(std::string button)
{
	auto range = mapped_inputs.equal_range(button);
	for (auto i = range.first; i != range.second; ++i)
	{
		if (keyboard_state[i->second] && !previous_keyboard_state[i->second])
		{
			return true;
		}
	}

	return false;
}

bool input_button_pressed(std::string button) {
	auto range = mapped_inputs.equal_range(button);
	for (auto i = range.first; i != range.second; ++i) {
		if (keyboard_state[i->second]) {
			return true;
		}
	}

	return false;
}

bool input_button_released(std::string button) {
	auto range = mapped_inputs.equal_range(button);
	for (auto i = range.first; i != range.second; ++i) {
		if (!keyboard_state[i->second] && previous_keyboard_state[i->second]) {
			return true;
		}
	}

	return false;
}

bool input_key_down(SDL_Scancode key) {
	return keyboard_state[key] && !previous_keyboard_state[key];
}

bool input_key_pressed(SDL_Scancode key) {
	return keyboard_state[key];
}

bool input_key_released(SDL_Scancode key) {
	return !keyboard_state[key] && previous_keyboard_state[key];
}

bool input_mouse_button_down(mouse_button button) {
	Uint32 mask = 0;

	switch (button) {
	case mouse_button::MOUSE_LEFT: mask = SDL_BUTTON_LMASK; break;
	case mouse_button::MOUSE_RIGHT: mask = SDL_BUTTON_RMASK; break;
	case mouse_button::MOUSE_MIDDLE: mask = SDL_BUTTON_MMASK; break;
	case mouse_button::MOUSE_BACK: mask = SDL_BUTTON_X1MASK; break;
	case mouse_button::MOUSE_FORWARD: mask = SDL_BUTTON_X2MASK; break;
	}

	return (mouse_state & mask) && !(previous_mouse_state & mask);
}

bool input_mouse_button_pressed(mouse_button button) {
	Uint32 mask = 0;

	switch (button) {
	case mouse_button::MOUSE_LEFT: mask = SDL_BUTTON_LMASK; break;
	case mouse_button::MOUSE_RIGHT: mask = SDL_BUTTON_RMASK; break;
	case mouse_button::MOUSE_MIDDLE: mask = SDL_BUTTON_MMASK; break;
	case mouse_button::MOUSE_BACK: mask = SDL_BUTTON_X1MASK; break;
	case mouse_button::MOUSE_FORWARD: mask = SDL_BUTTON_X2MASK; break;
	}

	return (mouse_state & mask);
}

bool input_mouse_button_released(mouse_button button) {
	Uint32 mask = 0;

	switch (button) {
	case mouse_button::MOUSE_LEFT: mask = SDL_BUTTON_LMASK; break;
	case mouse_button::MOUSE_RIGHT: mask = SDL_BUTTON_RMASK; break;
	case mouse_button::MOUSE_MIDDLE: mask = SDL_BUTTON_MMASK; break;
	case mouse_button::MOUSE_BACK: mask = SDL_BUTTON_X1MASK; break;
	case mouse_button::MOUSE_FORWARD: mask = SDL_BUTTON_X2MASK; break;
	}

	return !(mouse_state & mask) && (previous_mouse_state & mask);
}

const glm::ivec2 input_get_mouse_pos() {
	return mouse_position;
}

const glm::ivec2 input_get_previous_mouse_pos() {
	return previous_mouse_position;
}

const glm::ivec2 input_get_mouse_delta() {
	return previous_mouse_position - mouse_position;
}

const glm::ivec2 input_get_relative_mouse_pos() {
	// return mouse pos relative to resolution;
	glm::ivec2 relative_mouse_pos;
	relative_mouse_pos.x = mouse_position.x / (screen_resolution_x / 2) - 1;
	relative_mouse_pos.y = mouse_position.y / (screen_resolution_y / 2) - 1;

	return relative_mouse_pos;
}

const glm::ivec2 input_get_relative_previous_mouse_pos() {
	// return previous mouse pos relative to resolution;
 	glm::ivec2 previous_relative_mouse_pos;
	previous_relative_mouse_pos.x = previous_mouse_position.x / (screen_resolution_x / 2) - 1;
	previous_relative_mouse_pos.y = previous_mouse_position.y / (screen_resolution_y / 2) - 1;

	return previous_relative_mouse_pos;
}

const glm::ivec2 input_get_relative_mouse_delta() {
	// return mouse delta relative to resolution;
	glm::ivec2 relative_mouse_pos;
	relative_mouse_pos.x = mouse_position.x / screen_resolution_x - 1;
	relative_mouse_pos.y = mouse_position.y / screen_resolution_y - 1;

 	glm::ivec2 previous_relative_mouse_pos;
	previous_relative_mouse_pos.x = previous_mouse_position.x / screen_resolution_x - 1;
	previous_relative_mouse_pos.y = previous_mouse_position.y / screen_resolution_y - 1;

	return previous_relative_mouse_pos - relative_mouse_pos;
}

void save_input() {
	std::ofstream ofs("inputSettings.input");
	if (ofs.is_open()) {
		for (auto input : mapped_inputs) {
			ofs << input.first << " " << input.second << std::endl;
		}
	}

	ofs.close();
	log_info("SAVED INPUT");
}

void load_input() {
	mapped_inputs.clear();

	log_info("RETRIEVING INPUT FILE");
	std::ifstream ifs("inputSettings.input");
	if (ifs.is_open()) {
		log_info("PARSING INPUT FILE");

		std::string line;
		while (std::getline(ifs, line)) {
			std::string key;
			unsigned int value;

			std::istringstream iss(line);
			if (!(iss >> key >> value)) { break; }

			mapped_inputs.insert(std::make_pair(key, value));
			log_info(" - ", key.c_str(), " ", value);
		}
		log_info("FINISHED LOADING INPUT DATA");
	}
	else {
		// THIS IS CALLED IF NOT INPUT FILE EXISTS AND IT MAKES ONE.

		log_warning("CANNOT FIND INPUT FILE");

		//
		// -- MOVEMENT INPUT --
		//

		mapped_inputs.insert({ "MoveUp", SDL_SCANCODE_W });
		mapped_inputs.insert({ "MoveDown", SDL_SCANCODE_S });
		mapped_inputs.insert({ "MoveLeft", SDL_SCANCODE_A });
		mapped_inputs.insert({ "MoveRight", SDL_SCANCODE_D });

		mapped_inputs.insert({ "Run", SDL_SCANCODE_LSHIFT });

		//
		// -- COMBAT INPUT --
		//

		mapped_inputs.insert({ "Attack", SDL_SCANCODE_SPACE });
		mapped_inputs.insert({ "Shoot", SDL_SCANCODE_F });

		//
		// -- MISC INPUT --
		//

		mapped_inputs.insert({ "Exit", SDL_SCANCODE_ESCAPE });

		save_input();
		load_input();
	}

	ifs.close();
}
