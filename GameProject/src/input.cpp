#include "input.h"

#include <fstream>
#include <sstream>
#include <map>
#include <memory>

#include "renderer.h"
#include "log.h"

const Uint8* keyboard_state;
Uint8* previous_keyboard_state;
int keyboard_state_size;

Uint32 mouse_state;
Uint32 previous_mouse_state;
glm::ivec2 mouse_position;
glm::ivec2 previous_mouse_position;

void input::init() {
	keyboard_state = SDL_GetKeyboardState(&keyboard_state_size);
	previous_keyboard_state = new Uint8[keyboard_state_size];
	memcpy(previous_keyboard_state, keyboard_state, keyboard_state_size);

	logger::info("INITIALIZED INPUT");
}

void input::update() {
	previous_mouse_state = mouse_state;
	previous_mouse_position = mouse_position;
	mouse_state = SDL_GetMouseState(&mouse_position.x, &mouse_position.y);
	memcpy(previous_keyboard_state, keyboard_state, keyboard_state_size);
}

void input::clean() {
	delete[] previous_keyboard_state;
	previous_keyboard_state = NULL;
}

// TODO: Add Controller Support
enum input_type { KEYBOARD = 0, MOUSE };

struct INPUT {
	unsigned int input_index;
	input_type type;

	INPUT(Uint8 _input, input_type _type) : input_index(_input), type(_type) {}
	~INPUT() {}

	virtual bool State() {
		if (type == KEYBOARD) {
			return keyboard_state[input_index];
		} else if (type == MOUSE) {
			Uint32 mask = 0;

			switch (input_index) {
			case input::MOUSE_LEFT: mask = SDL_BUTTON_LMASK; break;
			case input::MOUSE_RIGHT: mask = SDL_BUTTON_RMASK; break;
			case input::MOUSE_MIDDLE: mask = SDL_BUTTON_MMASK; break;
			case input::MOUSE_BACK: mask = SDL_BUTTON_X1MASK; break;
			case input::MOUSE_FORWARD: mask = SDL_BUTTON_X2MASK; break;
			}

			return (mouse_state & mask);
		}

		return false;
	}
	virtual bool P_State() {
		if (type == KEYBOARD) {
			return previous_keyboard_state[input_index];
		} else if (type == MOUSE) {
			Uint32 mask = 0;

			switch (input_index) {
			case input::MOUSE_LEFT: mask = SDL_BUTTON_LMASK; break;
			case input::MOUSE_RIGHT: mask = SDL_BUTTON_RMASK; break;
			case input::MOUSE_MIDDLE: mask = SDL_BUTTON_MMASK; break;
			case input::MOUSE_BACK: mask = SDL_BUTTON_X1MASK; break;
			case input::MOUSE_FORWARD: mask = SDL_BUTTON_X2MASK; break;
			}

			return (previous_mouse_state & mask);
		}

		return false;
	}
};

static std::multimap<std::string, INPUT> MAPPED_INPUTS;

bool input::button_down(std::string button) {
	auto range = MAPPED_INPUTS.equal_range(button);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second.State() && !i->second.P_State()) {
			return true;
		}
	}

	return false;
}

bool input::button_pressed(std::string button) {
	auto range = MAPPED_INPUTS.equal_range(button);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second.State()) {
			return true;
		}
	}

	return false;
}

bool input::button_released(std::string button) {
	auto range = MAPPED_INPUTS.equal_range(button);
	for (auto i = range.first; i != range.second; ++i) {
		if (!i->second.State() && i->second.P_State()) {
			return true;
		}
	}

	return false;
}

bool input::key_down(SDL_Scancode key) {
	return keyboard_state[key] && !previous_keyboard_state[key];
}

bool input::key_pressed(SDL_Scancode key) {
	return keyboard_state[key];
}

bool input::key_released(SDL_Scancode key) {
	return !keyboard_state[key] && previous_keyboard_state[key];
}

bool input::mouse_button_down(mouse_button button) {
	Uint32 mask = 0;

	switch (button) {
	case input::MOUSE_LEFT: mask = SDL_BUTTON_LMASK; break;
	case input::MOUSE_RIGHT: mask = SDL_BUTTON_RMASK; break;
	case input::MOUSE_MIDDLE: mask = SDL_BUTTON_MMASK; break;
	case input::MOUSE_BACK: mask = SDL_BUTTON_X1MASK; break;
	case input::MOUSE_FORWARD: mask = SDL_BUTTON_X2MASK; break;
	}

	return (mouse_state & mask) && !(previous_mouse_state & mask);
}

bool input::mouse_button_pressed(mouse_button button) {
	Uint32 mask = 0;

	switch (button) {
	case input::MOUSE_LEFT: mask = SDL_BUTTON_LMASK; break;
	case input::MOUSE_RIGHT: mask = SDL_BUTTON_RMASK; break;
	case input::MOUSE_MIDDLE: mask = SDL_BUTTON_MMASK; break;
	case input::MOUSE_BACK: mask = SDL_BUTTON_X1MASK; break;
	case input::MOUSE_FORWARD: mask = SDL_BUTTON_X2MASK; break;
	}

	return (mouse_state & mask);
}

bool input::mouse_button_released(mouse_button button) {
	Uint32 mask = 0;

	switch (button) {
	case input::MOUSE_LEFT: mask = SDL_BUTTON_LMASK; break;
	case input::MOUSE_RIGHT: mask = SDL_BUTTON_RMASK; break;
	case input::MOUSE_MIDDLE: mask = SDL_BUTTON_MMASK; break;
	case input::MOUSE_BACK: mask = SDL_BUTTON_X1MASK; break;
	case input::MOUSE_FORWARD: mask = SDL_BUTTON_X2MASK; break;
	}

	return !(mouse_state & mask) && (previous_mouse_state & mask);
}

const glm::ivec2* input::get_mouse_pos() {
	return &mouse_position;
}

const glm::ivec2* input::get_previous_mouse_pos() {
	return &previous_mouse_position;
}

const glm::ivec2 input::get_mouse_delta() {
	return previous_mouse_position - mouse_position;
}

const glm::ivec2 input::get_relative_mouse_pos() {
	// return mouse pos relative to resolution;
	glm::ivec2 relative_mouse_pos;
	relative_mouse_pos.x = mouse_position.x / (renderer::screen_resolution_x / 2) - 1;
	relative_mouse_pos.y = mouse_position.y / (renderer::screen_resolution_y / 2) - 1;

	return relative_mouse_pos;
}

const glm::ivec2 input::get_relative_previous_mouse_pos() {
	// return previous mouse pos relative to resolution;
 	glm::ivec2 previous_relative_mouse_pos;
	previous_relative_mouse_pos.x = previous_mouse_position.x / (renderer::screen_resolution_x / 2) - 1;
	previous_relative_mouse_pos.y = previous_mouse_position.y / (renderer::screen_resolution_y / 2) - 1;

	return previous_relative_mouse_pos;
}

const glm::ivec2 input::get_relative_mouse_delta() {
	// return mouse delta relative to resolution;
	glm::ivec2 relative_mouse_pos;
	relative_mouse_pos.x = mouse_position.x / renderer::screen_resolution_x - 1;
	relative_mouse_pos.y = mouse_position.y / renderer::screen_resolution_y - 1;

 	glm::ivec2 previous_relative_mouse_pos;
	previous_relative_mouse_pos.x = previous_mouse_position.x / renderer::screen_resolution_x - 1;
	previous_relative_mouse_pos.y = previous_mouse_position.y / renderer::screen_resolution_y - 1;

	return previous_relative_mouse_pos - relative_mouse_pos;
}

void input::save() {
	std::ofstream ofs("inputSettings.input");
	if (ofs.is_open()) {
		for (auto KEY : MAPPED_INPUTS) {
			ofs << KEY.first << " " << KEY.second.type << " " << KEY.second.input_index << std::endl;
		}
	}

	ofs.close();
	logger::info("SAVED INPUT");
}

void input::load() {
	MAPPED_INPUTS.clear();

	logger::info("RETRIEVING INPUT FILE");
	std::ifstream ifs("inputSettings.input");
	if (ifs.is_open()) {
		logger::info("PARSING INPUT FILE");

		std::string line;
		while (std::getline(ifs, line)) {
			std::string key;
			unsigned int value;
			int type;

			std::istringstream iss(line);
			if (!(iss >> key >> type >> value)) { break; }

			MAPPED_INPUTS.insert(std::make_pair(key, INPUT(value, (input_type)type)));
			logger::info(" - ", key.c_str(), " ", value);
		}
		logger::info("FINISHED LOADING INPUT DATA");
	}
	else {
		// THIS IS CALLED IF NOT INPUT FILE EXISTS AND IT MAKES ONE.

		logger::warning("CANNOT FIND INPUT FILE");

		//
		// -- MOVEMENT INPUT --
		//

		MAPPED_INPUTS.insert({ "MoveUp", INPUT(SDL_SCANCODE_W, KEYBOARD)});
		MAPPED_INPUTS.insert({ "MoveDown", INPUT(SDL_SCANCODE_S, KEYBOARD) });
		MAPPED_INPUTS.insert({ "MoveLeft", INPUT(SDL_SCANCODE_A, KEYBOARD) });
		MAPPED_INPUTS.insert({ "MoveRight", INPUT(SDL_SCANCODE_D, KEYBOARD) });

		MAPPED_INPUTS.insert({ "Run", INPUT(SDL_SCANCODE_LSHIFT, KEYBOARD) });

		//
		// -- COMBAT INPUT --
		//

		MAPPED_INPUTS.insert({ "Attack", INPUT(SDL_SCANCODE_SPACE, KEYBOARD) });
		MAPPED_INPUTS.insert({ "Shoot", INPUT(SDL_SCANCODE_F, KEYBOARD) });

		//
		// -- MISC INPUT --
		//

		MAPPED_INPUTS.insert({ "Exit", INPUT(SDL_SCANCODE_ESCAPE, KEYBOARD) });

		save();
		load();
	}

	ifs.close();
}
