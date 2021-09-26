#include <SDL.h>

#ifdef _DEBUG
#include "imgui_impl_sdl.h"
#endif // _DEBUG

#include "scene.h"

#include "renderer.h"
#include "input.h"

#ifdef _DEBUG
#include "editor.h"
#endif // _DEBUG

#include "player.h"

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
		input::load();

		player::init();

		level::init();

#ifdef NDEBUG
		CurrentState = GAME_STATE::GAMEPLAY;
		level::load();
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
			CurrentState = (CurrentState == GAME_STATE::GAMEPLAY)
										  ? GAME_STATE::EDITOR
										  : GAME_STATE::GAMEPLAY;
		}

		if (CurrentState == GAME_STATE::GAMEPLAY) {
			player::update(dt);
			level::update(dt);
		}
		else if (CurrentState == GAME_STATE::EDITOR)
			editor::update(dt);
#endif // _DEBUG

#ifdef NDEBUG
		player::update(dt);
		level::update(dt);
#endif // NDEBUG

		input::update();

		/* ----- RENDER GAME ----- */
		renderer::start_draw();

		renderer::draw_sprites();

#ifdef _DEBUG
		if (CurrentState == GAME_STATE::EDITOR)
			editor::draw();
#endif // _DEBUG

		renderer::stop_draw();
	}

	/* ----- CLEAN GAME ----- */
	player::clean();

#ifdef _DEBUG
	editor::clean();
#endif // _DEBUG

	renderer::clean();

	return 0;
}