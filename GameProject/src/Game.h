#pragma once
// sdl and glew
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>

// dear imgui
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <string>

class Game {
public:
	Game();
	~Game();

	void init(const char* title);
	void handleEvents();
	void update();
	bool running() { return isRunning; }
	void render();
	void clean();

	static SDL_Renderer* renderer;
	static SDL_Event event;
	static double deltaTime;

	static const int width = 800;
	static const int height = 600;
private:
	bool isRunning;
	SDL_Window* window;
	SDL_GLContext context;
};