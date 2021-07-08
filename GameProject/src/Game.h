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

#include "Scene.h"

class Game {
public:
	void init(const char* title);
	void handleEvents();
	void update();
	void render();
	void clean();

	bool running() { return isRunning; }

	static int screen_width, screen_height;
	static const int screen_resolution_x, screen_resolution_y;

	static glm::mat4 projection;
	static glm::mat4 view;

	static SDL_Event event;
	static SDL_Window* window;
	static SDL_GLContext context;

	static Scene scene;
private:
	bool isRunning = false;
};