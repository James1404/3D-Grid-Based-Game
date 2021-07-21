#pragma once
#include <functional>
#include <map>
#include <string>
#include <SDL.h>
#include <json.hpp>
#include <fstream>
#include <iostream>

class Input {
private:
	std::multimap<std::string, SDL_Scancode> MAPPED_KEYS;
	
	Uint8* mPrevKeyboardState;
	const Uint8* mKeyboardState;
	int mKeyLength;

	Input();
	~Input();
public:
	void UpdatePrevInput();

	bool ButtonDown(std::string button);
	bool ButtonPressed(std::string button);
	bool ButtonReleased(std::string button);

	void SaveInput();
	void LoadInput();

	static Input& instance() {
		static Input* instance = new Input();
		return *instance;
	}
};