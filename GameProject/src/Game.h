#pragma once
// sdl and glew
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>

// glm
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// dear imgui
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

// stb_image
#include "stb_image.h"

#include <string>

enum class GameState {
	GameState_Game,
	GameState_Menu,
	GameState_Edit
};

class Game {
public:
	Game() { }
	~Game() { }

	void init(const char* title);
	void handleEvents();
	void update();
	bool running() { return isRunning; }
	void render();
	void clean();

	static int Width, Height;
	static int ResolutionX, ResolutionY;

	static glm::mat4 projection;
	static glm::mat4 view;

	static SDL_Event event;

	static GameState gameState;
private:
	bool isRunning = false;
	SDL_Window* window;
	SDL_GLContext context;
};