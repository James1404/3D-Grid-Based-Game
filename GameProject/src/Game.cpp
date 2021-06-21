#include "Game.h"
#include "Entities/EditmodeCamera.h"
#include "Scene.h"

glm::mat4 Game::projection;
glm::mat4 Game::view = glm::mat4(1.0f);

GameState Game::gameState = GameState::GameState_Game;

SDL_Event Game::event;

int Game::Width = 1280;
int Game::Height = 720;

int Game::ResolutionX = 320;
int Game::ResolutionY = 200;

EditmodeCamera camera;
Scene scene;

void Game::init(const char* title) {
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		
		const char* glsl_version = "#version 330";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		context = SDL_GL_CreateContext(window);
		glViewport(0, 0, Width, Height);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		if (context != NULL) {
			glewExperimental = GL_TRUE;
			GLenum glewError = glewInit();
			if (glewError != GLEW_OK) {
				printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
			}
		}

		projection = glm::ortho(0.0f, static_cast<float>(ResolutionX), 0.0f, static_cast<float>(ResolutionY), -1.0f, 1.0f);

		// Initialize Entites
		scene.loadScene();

		// Initialize Imgui
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplSDL2_InitForOpenGL(window, context);
		ImGui_ImplOpenGL3_Init(glsl_version);

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
		case SDL_WINDOWEVENT_RESIZED:
			Width = event.window.data1;
			Height = event.window.data2;
			SDL_SetWindowSize(window, Width, Height);
			glViewport(0, 0, Width, Height);
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
	}
}

Uint64 NOW = SDL_GetPerformanceCounter();
Uint64 LAST = 0;
void Game::update() {
	// calculate delta time;
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();

	double dt = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

	if (gameState == GameState::GameState_Edit) {
		camera.update(dt);
	}
	else {
		scene.update(dt);
	}
}

void Game::render() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	scene.render();

	if (gameState == GameState::GameState_Edit) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

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