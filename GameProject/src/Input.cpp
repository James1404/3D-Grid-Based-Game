#include "Input.h"

Input::Input() {
	MAPPED_KEYS.insert(std::make_pair("Exit",		SDL_SCANCODE_ESCAPE));
	MAPPED_KEYS.insert(std::make_pair("MoveUp",		SDL_SCANCODE_W));
	MAPPED_KEYS.insert(std::make_pair("MoveDown",	SDL_SCANCODE_S));
	MAPPED_KEYS.insert(std::make_pair("MoveLeft",	SDL_SCANCODE_A));
	MAPPED_KEYS.insert(std::make_pair("MoveRight",	SDL_SCANCODE_D));
	MAPPED_KEYS.insert(std::make_pair("Run",		SDL_SCANCODE_LSHIFT));
	MAPPED_KEYS.insert(std::make_pair("Shoot",		SDL_SCANCODE_SPACE));

	mKeyboardState = SDL_GetKeyboardState(&mKeyLength);
	mPrevKeyboardState = new Uint8[mKeyLength];
	memcpy(mPrevKeyboardState, mKeyboardState, mKeyLength);
}

Input::~Input() {
	delete[] mPrevKeyboardState;
	mPrevKeyboardState = NULL;
}

void Input::UpdatePrevInput() {
	memcpy(mPrevKeyboardState, mKeyboardState, mKeyLength);
}

bool Input::ButtonDown(std::string button) {
	auto range = MAPPED_KEYS.equal_range(button);
	for (auto KEY = range.first; KEY != range.second; KEY++) {
		if (mKeyboardState[KEY->second] && !mPrevKeyboardState[KEY->second]) {
			return true;
		}
	}

	return false;
}

bool Input::ButtonPressed(std::string button) {
	auto range = MAPPED_KEYS.equal_range(button);
	for (auto KEY = range.first; KEY != range.second; KEY++) {
		if (mKeyboardState[KEY->second]) {
			return true;
		}
	}

	return false;
}

bool Input::ButtonReleased(std::string button) {
	auto range = MAPPED_KEYS.equal_range(button);
	for (auto KEY = range.first; KEY != range.second; KEY++) {
		if (!mKeyboardState[KEY->second] && mPrevKeyboardState[KEY->second]) {
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