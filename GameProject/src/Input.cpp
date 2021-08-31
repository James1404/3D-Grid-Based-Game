#include "Input.h"

#include <fstream>
#include <sstream>

Input::Input() {
	KEYBOARD = SDL_GetKeyboardState(&KEYBOARD_SIZE);
	P_KEYBOARD = new Uint8[KEYBOARD_SIZE];
	memcpy(P_KEYBOARD, KEYBOARD, KEYBOARD_SIZE);
}

Input::~Input() {
	delete[] P_KEYBOARD;
	P_KEYBOARD = NULL;
}

void UpdatePrevInput() {
	memcpy(m_input.P_KEYBOARD, m_input.KEYBOARD, m_input.KEYBOARD_SIZE);
}

bool ButtonDown(std::string button) {
	auto range = m_input.MAPPED_KEYS.equal_range(button);
	for (auto KEY = range.first; KEY != range.second; KEY++) {
		if (m_input.KEYBOARD[KEY->second] && !m_input.P_KEYBOARD[KEY->second]) {
			return true;
		}
	}

	return false;
}

bool ButtonPressed(std::string button) {
	auto range = m_input.MAPPED_KEYS.equal_range(button);
	for (auto KEY = range.first; KEY != range.second; KEY++) {
		if (m_input.KEYBOARD[KEY->second]) {
			return true;
		}
	}

	return false;
}

bool ButtonReleased(std::string button) {
	auto range = m_input.MAPPED_KEYS.equal_range(button);
	for (auto KEY = range.first; KEY != range.second; KEY++) {
		if (!m_input.KEYBOARD[KEY->second] && m_input.P_KEYBOARD[KEY->second]) {
			return true;
		}
	}

	return false;
}

void SaveInput() {
	std::ofstream ofs("inputSettings.input");
	if (ofs.is_open()) {
		for (auto KEY : m_input.MAPPED_KEYS) {
			ofs << KEY.first << " " << KEY.second << std::endl;
		}
	}
	else {
		printf("Cannot open input settings file\n");
	}

	ofs.close();
	printf("Input Settings Saved\n");
}

// TODO: Finish Loading Function
void LoadInput() {
	m_input.MAPPED_KEYS.clear();

	std::ifstream ifs("inputSettings.input");
	if (ifs.is_open()) {
		std::string line;
		while (std::getline(ifs, line)) {
			std::string key;
			unsigned int value;

			std::istringstream iss(line);
			if (!(iss >> key >> value)) { break; }

			printf("%s : %u\n", key.c_str(), value);
			m_input.MAPPED_KEYS.insert(std::make_pair(key, (SDL_Scancode)value));
		}
	}
	else {
		printf("Cannot open input settings file\n");

		m_input.MAPPED_KEYS.insert(std::make_pair("MoveUp", SDL_SCANCODE_W));
		m_input.MAPPED_KEYS.insert(std::make_pair("MoveDown", SDL_SCANCODE_S));
		m_input.MAPPED_KEYS.insert(std::make_pair("MoveLeft", SDL_SCANCODE_A));
		m_input.MAPPED_KEYS.insert(std::make_pair("MoveRight", SDL_SCANCODE_D));
		m_input.MAPPED_KEYS.insert(std::make_pair("Run", SDL_SCANCODE_LSHIFT));
		m_input.MAPPED_KEYS.insert(std::make_pair("Aim", SDL_SCANCODE_LCTRL));
		m_input.MAPPED_KEYS.insert(std::make_pair("Shoot", SDL_SCANCODE_SPACE));
		m_input.MAPPED_KEYS.insert(std::make_pair("Exit", SDL_SCANCODE_ESCAPE));

		SaveInput();
	}

	ifs.close();
	printf("Input Settings Loaded\n");
}