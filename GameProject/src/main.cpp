#include <SDL.h>

#include "renderer.h"
#include "input.h"
#include "log.h"
#include "editor.h"

int main(int argc, char* args[]) {
	SDL_Event event;

	bool isRunning = false;

	entity_manager entities;
#ifdef _DEBUG
	editor_manager editor;
#endif // _DEBUG

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

#ifdef _DEBUG
		editor.init(entities);
#endif // _DEBUG
		
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
					renderer::screen_resolution_x = event.window.data1;
					renderer::screen_resolution_y = event.window.data2;
					SDL_SetWindowSize(renderer::window, renderer::screen_resolution_x, renderer::screen_resolution_y);
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

#ifdef _DEBUG
		if (CurrentState == GAME_STATE::EDITOR) {
			entities.is_paused = true;
			editor.update(dt);
		}
		else {
			entities.is_paused = false;
		}
#endif // _DEBUG

		if (input::key_down(SDL_SCANCODE_ESCAPE)) {
			if (CurrentState == GAME_STATE::GAMEPLAY)
				CurrentState = GAME_STATE::EDITOR;
			else if (CurrentState == GAME_STATE::EDITOR)
				CurrentState = GAME_STATE::GAMEPLAY;
		}

		/* ----- RENDER GAME ----- */
		renderer::start_draw();
		
		renderer::draw_sprites();

#ifdef _DEBUG
		if (CurrentState == GAME_STATE::EDITOR) {
			editor.draw();
		}
#endif // _DEBUG

		renderer::debug::draw_debug();

		renderer::stop_draw();

		input::update();
	}

	/* ----- CLEAN GAME ----- */
	logger::info("STARTING CLEANUP");

#ifdef _DEBUG
	editor.clean();
#endif // _DEBUG

	entities.clean();

	renderer::debug::clean_debug();
	renderer::clean();

	logger::info("CLEANUP FINISHED");

	return 0;
}
