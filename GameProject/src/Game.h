#pragma once
#include <SDL.h>

class Game {
public:
	Game();
	~Game();

	void init(const char* title, int width, int height, bool fullscreen);
	void handleEvents();
	void update();
	bool running() { return isRunning; }
	void render();
	void clean();

	static SDL_Renderer* renderer;
	static SDL_Event event;
	static double deltaTime;
private:
	bool isRunning;
	SDL_Window* window;
};