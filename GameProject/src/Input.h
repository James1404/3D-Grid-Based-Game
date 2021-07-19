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
	std::multimap<std::string, SDL_KeyCode> MAPPED_KEYS;
	
	bool KEYS[322];
	bool P_KEYS[322];

	Input();
	~Input();
public:
	void UpdateInput(SDL_Event& e);

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