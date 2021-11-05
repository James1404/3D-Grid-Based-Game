#include <SDL.h>

#include "scene.h"

#include "renderer.h"
#include "input.h"

#include "player.h"
#include "camera.h"

// TODO: implement centrilaised log system

SDL_Event event;

bool isRunning = false;

enum class GAME_STATE {
	GAMEPLAY,
	MENU
} static CurrentState;

uint64_t NOW = SDL_GetPerformanceCounter(), LAST = 0;
int main(int argc, char* args[]) {
	/* ----- INIT GAME ----- */
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		renderer::init();

		input::init();
		input::load();

		CurrentState = GAME_STATE::GAMEPLAY;

		// LEVEL NAMES :
		// combattestlevel
		// newtestlevel

		current_level.load("combattestlevel");
		current_level.init();
		
		player = std::make_shared<player_entity>();

		isRunning = true;
	}

	while (isRunning) {
		/* ----- HANDLE EVENTS ----- */
		if (SDL_PollEvent(&event)) {
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

		if (CurrentState == GAME_STATE::GAMEPLAY) {
			player->update(dt);
			current_level.update(dt);
		}

		camera::update(dt);

		input::update();

		/* ----- RENDER GAME ----- */
		renderer::start_draw();

		renderer::draw_sprites();

		renderer::stop_draw();
	}

	/* ----- CLEAN GAME ----- */
	printf("----------------\n");
	printf("STARTING CLEANUP\n");

	current_level.clean();

	renderer::clean();

	printf("CLEANUP FINISHED\n");

	return 0;
}
