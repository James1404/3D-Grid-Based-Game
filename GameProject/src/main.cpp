#include <SDL.h>

#include "scene.h"

#include "renderer.h"
#include "input.h"

#ifdef _DEBUG
#include <imgui_impl_sdl.h>
#include "editor.h"
#endif // _DEBUG

#include "player.h"
#include "camera.h"

// TODO: implement centrilaised log system

SDL_Event event;

bool isRunning = false;

enum class GAME_STATE {
	GAMEPLAY,
#ifdef _DEBUG
	EDITOR
#endif // _DEBUG
} static CurrentState;

Uint64 NOW = SDL_GetPerformanceCounter(), LAST = 0;
int main(int argc, char* args[]) {
	/* ----- INIT GAME ----- */
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		renderer::init();

		input::init();
		input::load();

		player::init();

		level::init();

#ifdef NDEBUG
		CurrentState = GAME_STATE::GAMEPLAY;
		level::load("testlevel");
#endif // NDEBUG
		
#ifdef _DEBUG
		CurrentState = GAME_STATE::EDITOR;
		editor::init();
#endif // _DEBUG

		isRunning = true;
	}

	while (isRunning) {
		/* ----- HANDLE EVENTS ----- */
		if (SDL_PollEvent(&event)) {
#ifdef _DEBUG
			ImGui_ImplSDL2_ProcessEvent(&event);
#endif // _DEBUG
			switch (event.type) {
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_WINDOWEVENT:
				// TODO: Screen doesnt scale properly.
				if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					renderer::screen_width = event.window.data1;
					renderer::screen_height = event.window.data2;
					SDL_SetWindowSize(renderer::window, renderer::screen_width, renderer::screen_height);
				}
				break;
			default:
				break;
			}
		}

		/* ----- UPDATE GAME ----- */
		// calculate delta time;
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		double dt = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

#ifdef _DEBUG
		if (input::button_down("Exit")) {
			if (CurrentState == GAME_STATE::EDITOR) {
				CurrentState = GAME_STATE::GAMEPLAY;
			}
			else if (CurrentState == GAME_STATE::GAMEPLAY) {
				level::load(level::data.name);
				editor::clear_selected();
				CurrentState = GAME_STATE::EDITOR;
			}
		}

		if (CurrentState == GAME_STATE::GAMEPLAY) {
			player::update(dt);
			level::update(dt);
		}

#endif // _DEBUG

#ifdef NDEBUG
		player::update(dt);
		level::update(dt);
#endif // NDEBUG

		camera::update(dt);

		input::update();

		/* ----- RENDER GAME ----- */
		renderer::start_draw();

		renderer::draw_sprites();

#ifdef _DEBUG
		for (auto node : level::data.path_nodes) {
			glm::vec3 colour = renderer::COLOUR::CYAN;

			if (node->combat_node)
				colour = renderer::COLOUR::RED;

			renderer::debug::draw_square(node->pos, { 10,10 }, colour);
		}
#endif // _DEBUG

		

#ifdef _DEBUG
		if (CurrentState == GAME_STATE::EDITOR)
			editor::draw();
#endif // _DEBUG
		
		renderer::stop_draw();
	}

	/* ----- CLEAN GAME ----- */
	printf("----------------\n");
	printf("STARTING CLEANUP\n");

#ifdef _DEBUG
	editor::clean();
#endif // _DEBUG

	player::clean();
	level::clean();

	renderer::clean();

	printf("CLEANUP FINISHED\n");

	return 0;
}