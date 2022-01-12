#include <SDL.h>

#include "renderer.h"
#include "input.h"
#include "log.h"
#include "editor.h"

int main(int argc, char* args[]) {
	SDL_Event event;

	bool isRunning = false;

	entity_manager entities;
	editor_manager editor;

	enum class GAME_STATE {
		GAMEPLAY,
		EDITOR
	} static CurrentState;

	uint64_t NOW = SDL_GetPerformanceCounter(), LAST = 0;

	/* ----- INIT GAME ----- */
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		renderer::init();
		renderer::debug::init_debug();

		input::init();
		input::load();

		CurrentState = GAME_STATE::GAMEPLAY;

		// LEVEL NAMES :
		// combattestlevel
		// newtestlevel
		entities.load("combattestlevel");
		entities.init();

		editor.init(entities);
		
		isRunning = true;
	}

	while (isRunning) {
		/* ----- HANDLE EVENTS ----- */
		if (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				isRunning = false;
				break;
			/*
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					renderer::screen_width = event.window.data1;
					renderer::screen_height = event.window.data2;
					SDL_SetWindowSize(renderer::window, renderer::screen_width, renderer::screen_height);
				}
				break;
			*/
			}
		}

		/* ----- UPDATE GAME ----- */
		renderer::debug::clear_debug_list();

		// calculate delta time;
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		double dt = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

		entities.update(dt);

		if (CurrentState == GAME_STATE::EDITOR) {
			entities.is_paused = true;
			editor.update(dt);
		}
		else {
			entities.is_paused = false;
		}

		if (input::key_down(SDL_SCANCODE_ESCAPE)) {
			if (CurrentState == GAME_STATE::GAMEPLAY)
				CurrentState = GAME_STATE::EDITOR;
			else if (CurrentState == GAME_STATE::EDITOR)
				CurrentState = GAME_STATE::GAMEPLAY;
		}

		/* ----- RENDER GAME ----- */
		renderer::start_draw();
		
		renderer::draw_sprites();

		if (CurrentState == GAME_STATE::EDITOR) {
			editor.draw();
		}

		renderer::debug::draw_debug();

		renderer::stop_draw();

		input::update();
	}

	/* ----- CLEAN GAME ----- */
	logger::info("STARTING CLEANUP");

	entities.clean();

	renderer::debug::clean_debug();
	renderer::clean();

	logger::info("CLEANUP FINISHED");

	return 0;
}
