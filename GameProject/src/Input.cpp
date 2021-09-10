#include "Input.h"

#include <fstream>
#include <sstream>
#include <map>
#include <memory>

#include <SDL.h>

// TODO: Add Controller Support
struct InputData {
	const Uint8* KEYBOARD;
	Uint8* P_KEYBOARD;
	int KEYBOARD_SIZE;

	Uint32 MOUSE;
	Uint32 P_MOUSE;
	glm::ivec2 MOUSE_POS;

	InputData() {
		KEYBOARD = SDL_GetKeyboardState(&KEYBOARD_SIZE);
		P_KEYBOARD = new Uint8[KEYBOARD_SIZE];
		memcpy(P_KEYBOARD, KEYBOARD, KEYBOARD_SIZE);
	}

	~InputData() {
		delete[] P_KEYBOARD;
		P_KEYBOARD = NULL;
	}
} static INPUT_DATA;

enum MOUSE_BUTTON { LEFT = 0, RIGHT, MIDDLE, BACK, FORWARD };

enum INPUT_TYPE { KEYBOARD = 0, MOUSE };
struct INPUT {
	unsigned int input;
	INPUT_TYPE type;

	INPUT(Uint8 _input, INPUT_TYPE _type) : input(_input), type(_type) {}
	~INPUT() {}

	virtual bool State() {
		if (type == KEYBOARD) {
			return INPUT_DATA.KEYBOARD[input];
		} else if (type == MOUSE) {
			Uint32 mask = 0;

			switch (input) {
			case LEFT: mask = SDL_BUTTON_LMASK; break;
			case RIGHT: mask = SDL_BUTTON_RMASK; break;
			case MIDDLE: mask = SDL_BUTTON_MMASK; break;
			case BACK: mask = SDL_BUTTON_X1MASK; break;
			case FORWARD: mask = SDL_BUTTON_X2MASK; break;
			}

			return (INPUT_DATA.MOUSE & mask);
		}

		return false;
	}
	virtual bool P_State() {
		if (type == KEYBOARD) {
			return INPUT_DATA.P_KEYBOARD[input];
		} else if (type == MOUSE) {
			Uint32 mask = 0;

			switch (input) {
			case LEFT: mask = SDL_BUTTON_LMASK; break;
			case RIGHT: mask = SDL_BUTTON_RMASK; break;
			case MIDDLE: mask = SDL_BUTTON_MMASK; break;
			case BACK: mask = SDL_BUTTON_X1MASK; break;
			case FORWARD: mask = SDL_BUTTON_X2MASK; break;
			}

			return (INPUT_DATA.P_MOUSE & mask);
		}
	}
};

static std::multimap<std::string, INPUT> MAPPED_INPUTS;

void UpdateInput() {
	INPUT_DATA.P_MOUSE = INPUT_DATA.MOUSE;
	INPUT_DATA.MOUSE = SDL_GetMouseState(&INPUT_DATA.MOUSE_POS.x, &INPUT_DATA.MOUSE_POS.y);
	memcpy(INPUT_DATA.P_KEYBOARD, INPUT_DATA.KEYBOARD, INPUT_DATA.KEYBOARD_SIZE);
}

bool ButtonDown(std::string button) {
	auto range = MAPPED_INPUTS.equal_range(button);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second.State() && !i->second.P_State()) {
			return true;
		}
	}

	return false;
}

bool ButtonPressed(std::string button) {
	auto range = MAPPED_INPUTS.equal_range(button);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second.State()) {
			return true;
		}
	}

	return false;
}

bool ButtonReleased(std::string button) {
	auto range = MAPPED_INPUTS.equal_range(button);
	for (auto i = range.first; i != range.second; ++i) {
		if (!i->second.State() && i->second.P_State()) {
			return true;
		}
	}

	return false;
}

const glm::ivec2* GetMousePos() {
	return &INPUT_DATA.MOUSE_POS;
}

void SaveInput() {
	std::ofstream ofs("inputSettings.input");
	if (ofs.is_open()) {
		for (auto KEY : MAPPED_INPUTS) {
			ofs << KEY.first << " " << KEY.second.type << " " << KEY.second.input << std::endl;
		}
	}
	else {
		printf("Cannot open input settings file\n");
	}

	ofs.close();
	printf("Input Settings Saved\n");
}

void LoadInput() {
	MAPPED_INPUTS.clear();

	std::ifstream ifs("inputSettings.input");
	if (ifs.is_open()) {
		std::string line;
		while (std::getline(ifs, line)) {
			std::string key;
			unsigned int value;
			int type;

			std::istringstream iss(line);
			if (!(iss >> key >> type >> value)) { break; }

			printf("%s : %u\n", key.c_str(), value);

			MAPPED_INPUTS.insert(std::make_pair(key, INPUT(value, (INPUT_TYPE)type)));
		}
		printf("Input Settings Loaded\n");
	}
	else {
		printf("Cannot open input settings file\n");

		MAPPED_INPUTS.insert(std::make_pair("MoveUp", INPUT(SDL_SCANCODE_W, KEYBOARD)));
		MAPPED_INPUTS.insert(std::make_pair("MoveDown", INPUT(SDL_SCANCODE_S, KEYBOARD)));
		MAPPED_INPUTS.insert(std::make_pair("MoveLeft", INPUT(SDL_SCANCODE_A, KEYBOARD)));
		MAPPED_INPUTS.insert(std::make_pair("MoveRight", INPUT(SDL_SCANCODE_D, KEYBOARD)));
		MAPPED_INPUTS.insert(std::make_pair("Run", INPUT(SDL_SCANCODE_LSHIFT, KEYBOARD)));
		MAPPED_INPUTS.insert(std::make_pair("Aim", INPUT(SDL_SCANCODE_LCTRL, KEYBOARD)));
		MAPPED_INPUTS.insert(std::make_pair("Shoot", INPUT(SDL_SCANCODE_SPACE, KEYBOARD)));
		MAPPED_INPUTS.insert(std::make_pair("Exit", INPUT(SDL_SCANCODE_ESCAPE, KEYBOARD)));

		MAPPED_INPUTS.insert(std::make_pair("Shoot", INPUT(LEFT, MOUSE)));
		MAPPED_INPUTS.insert(std::make_pair("Aim", INPUT(RIGHT, MOUSE)));

		SaveInput();
		LoadInput();
	}

	ifs.close();
}