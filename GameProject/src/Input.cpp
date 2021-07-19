#include "Input.h"

Input::Input() {
	MAPPED_KEYS.insert(std::make_pair("MoveUp",		SDLK_w));
	MAPPED_KEYS.insert(std::make_pair("MoveDown",	SDLK_s));
	MAPPED_KEYS.insert(std::make_pair("MoveLeft",	SDLK_a));
	MAPPED_KEYS.insert(std::make_pair("MoveRight",	SDLK_d));
	MAPPED_KEYS.insert(std::make_pair("Shoot",		SDLK_SPACE));
}

Input::~Input() {

}

void Input::UpdateInput(SDL_Event& e) {
	memcpy(P_KEYS, KEYS, 322);
	if (e.type == SDL_KEYDOWN) {
		KEYS[e.key.keysym.sym] = true;
	}

	if (e.type == SDL_KEYUP) {
		KEYS[e.key.keysym.sym] = false;
	}
}

bool Input::ButtonDown(std::string button) {
	auto range = MAPPED_KEYS.equal_range(button);
	for (auto KEY = range.first; KEY != range.second; KEY++) {
		if (KEYS[KEY->second] && !P_KEYS[KEY->second]) {
			return true;
		}
	}

	return false;
}

bool Input::ButtonPressed(std::string button) {
	auto range = MAPPED_KEYS.equal_range(button);
	for (auto KEY = range.first; KEY != range.second; KEY++) {
		if (KEYS[KEY->second]) {
			return true;
		}
	}

	return false;
}

bool Input::ButtonReleased(std::string button) {
	auto range = MAPPED_KEYS.equal_range(button);
	for (auto KEY = range.first; KEY != range.second; KEY++) {
		if (!KEYS[KEY->second] && P_KEYS[KEY->second]) {
			return true;
		}
	}

	return false;
}

void Input::SaveInput() {
	nlohmann::json j;

	for (auto KEY : MAPPED_KEYS) {
		std::cout << KEY.first << " : " << KEY.second << std::endl;
		j[KEY.first] += KEY.second;
	}

	std::ofstream ofs("inputSettings.input");
	if (ofs.is_open()) {
		ofs << j.dump(4) << std::endl;
	}
	else {
		printf("Cannot open input settings file\n");
	}

	ofs.close();
	printf("Input Settings Saved\n");
}

void Input::LoadInput() {
	MAPPED_KEYS.clear();

	nlohmann::json j;
	std::ifstream ifs("inputSettings.input");
	if (ifs.is_open()) {
		j = nlohmann::json::parse(ifs);
	}
	else {
		printf("Cannot open input settings file\n");
	}

	ifs.close();

	printf("Input Settings Loaded\n");
}