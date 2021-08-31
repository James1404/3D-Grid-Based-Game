#pragma once
#include <map>
#include <string>
#include <SDL.h>

// TODO: Add Controller Support
// TODO: Add Mouse Support

struct Input {
	std::multimap<std::string, SDL_Scancode> MAPPED_KEYS;

	const Uint8* KEYBOARD;
	Uint8* P_KEYBOARD;
	int KEYBOARD_SIZE;

	Input();
	~Input();
};

static Input m_input;

void SaveInput();
void LoadInput();

void UpdatePrevInput();

bool ButtonDown(std::string button);
bool ButtonPressed(std::string button);
bool ButtonReleased(std::string button);