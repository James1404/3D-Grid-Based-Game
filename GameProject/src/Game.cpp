#pragma once
#include "Game.h"
#include "Entities/Editor.h"
#include "Input.h"

enum class GameState {
	GameState_Game,
	GameState_Menu,
	GameState_Edit
};

glm::mat4 Game::projection;
glm::mat4 Game::view = glm::mat4(1.0f);

GameState gameState = GameState::GameState_Game;

SDL_Event Game::event;
SDL_Window* Game::window;
SDL_GLContext Game::context;

int Game::screen_width = 1280;
int Game::screen_height = 720;

const int Game::screen_resolution_x = 320;
const int Game::screen_resolution_y = 200;

Scene Game::scene;

Editor editor;

void Game::init(const char* title) {
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		context = SDL_GL_CreateContext(window);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		if (context != NULL) {
			glewExperimental = GL_TRUE;
			GLenum glewError = glewInit();
			if (glewError != GLEW_OK) {
				printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
			}
		}

		projection = glm::ortho(0.0f, static_cast<float>(screen_resolution_x), 0.0f, static_cast<float>(screen_resolution_y), -1.0f, 1.0f);

		// Initialize Entites
		scene.init();
		editor.init();

		isRunning = true;
	}
}

void Game::handleEvents() {
	if (SDL_PollEvent(&event)) {
		ImGui_ImplSDL2_ProcessEvent(&event);
		switch (event.type) {
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				screen_width = event.window.data1;
				screen_height = event.window.data2;
				SDL_SetWindowSize(window, screen_width, screen_height);
			}
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				if (gameState == GameState::GameState_Game) {
					gameState = GameState::GameState_Edit;
					break;
				}

				gameState = GameState::GameState_Game;
			}
			break;
		default:
			break;
		}

		Input::instance().UpdateInput(event);
	}
}

Uint64 NOW = SDL_GetPerformanceCounter(), LAST = 0;
void Game::update() {
	// calculate delta time;
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();

	double dt = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

	if (gameState == GameState::GameState_Edit) {
		editor.update(dt);
	}
	else {
		scene.update(dt);
	}
}

void Game::render() {
	// Clear screen
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Set aspect ratio
	float x = screen_width / (float)screen_resolution_x;
	float y = screen_height / (float)screen_resolution_y;
	float aspect = std::min(x, y);

	int viewWidth = screen_resolution_x * aspect;
	int viewHeight = screen_resolution_y * aspect;

	int viewX = (screen_width - screen_resolution_x * aspect) / 2;
	int viewY = (screen_height - screen_resolution_y * aspect) / 2;

	glViewport(viewX, viewY, viewWidth, viewHeight);

	// render scene
	scene.render();

	// render imgui windows
	if (gameState == GameState::GameState_Edit) {
		editor.render();
	}

	// Swap buffer
	SDL_GL_SwapWindow(window);
}

void Game::clean() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}