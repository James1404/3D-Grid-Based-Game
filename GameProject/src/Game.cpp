#include "Game.h"
#include "Entity.h"
#include "Entities/Player.h"
#include <iostream>


SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event;
double Game::deltaTime;

Player* player;

Game::Game() { }
Game::~Game() { }

void Game::init(const char* title) {
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
		
		const char* glsl_version = "#version 330";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		context = SDL_GL_CreateContext(window);

		if (context != NULL) {
			glewExperimental = GL_TRUE;
			GLenum glewError = glewInit();
			if (glewError != GLEW_OK) {
				printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
			}
		}

		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplSDL2_InitForOpenGL(window, context);
		ImGui_ImplOpenGL3_Init(glsl_version);

		isRunning = true;
	}
	player = new Player();
}

void Game::handleEvents() {
	SDL_PollEvent(&event);
	ImGui_ImplSDL2_ProcessEvent(&event);
	if (event.type == SDL_QUIT) isRunning = false;
}

Uint64 NOW = SDL_GetPerformanceCounter();
Uint64 LAST = 0;
void Game::update() {
	// calculate delta time;
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();

	deltaTime = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());
	std::cout << deltaTime << std::endl;

	// update entities
	player->update();
}

void Game::render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Render();
	glViewport(0, 0, width, height);
	glClearColor(0.6f, 0.6f, 0.6f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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