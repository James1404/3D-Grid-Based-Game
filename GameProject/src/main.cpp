#include <SDL.h>

#include "renderer.h"
#include "input.h"
#include "log.h"

#include "player.h"
#include "camera.h"

// TODO: implement centrilaised log system

SDL_Event event;

bool isRunning = false;

enum class GAME_STATE {
	GAMEPLAY,
	MENU
} static CurrentState;

entity_manager manager;

uint64_t NOW = SDL_GetPerformanceCounter(), LAST = 0;

int main(int argc, char* args[]) {
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
		manager.load("combattestlevel");
		manager.init();
		
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

		if (CurrentState == GAME_STATE::GAMEPLAY) {
			manager.update(dt);
		}

		input::update();

		/* ----- RENDER GAME ----- */
		renderer::start_draw();

		renderer::draw_sprites();

		renderer::debug::draw_debug();

		renderer::stop_draw();
	}

	/* ----- CLEAN GAME ----- */
	logger::info("STARTING CLEANUP");

	manager.clean();

	renderer::debug::clean_debug();
	renderer::clean();

	logger::info("CLEANUP FINISHED");

	return 0;
}
