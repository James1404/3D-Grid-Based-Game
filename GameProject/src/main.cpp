#include <SDL.h>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>
#include <winpackagefamily.h>

#include "renderer.h"
#include "input.h"
#include "log.h"
#include "entity.h"
#include "editor.h"
#include "camera.h"
#include "event.h"

int main(int argc, char* args[])
{
	bool isRunning = true;

	SDL_Event event;

	entity_manager_t entity_manager;
	input_manager_t input_manager;
	camera_manager_t camera_manager;
	event_manager_t event_manager;
#ifdef _DEBUG
	editor_manager editor;
#endif // _DEBUG

	enum engine_state {
		engine_state_gameplay,
		engine_state_editor
	} current_engine_state;

	/* ----- INIT GAME ----- */
	renderer::init();
#ifdef _DEBUG
	renderer::debug::init_debug();
#endif // _DEBUG

	current_engine_state = engine_state_gameplay;

	// LEVEL NAMES :
	// combattestlevel
	// newtestlevel
	entity_manager.load("combattestlevel");

	input_manager.load();
#ifdef _DEBUG
	editor.init(entity_manager);
#endif // _DEBUG

	uint64_t NOW = SDL_GetPerformanceCounter(), LAST = 0;
	while (isRunning) {
		/* ----- HANDLE EVENTS ----- */
		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
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
#ifdef _DEBUG
		renderer::debug::clear_debug_list();
#endif // _DEBUG

		// calculate delta time;
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		double dt = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

		entity_manager.update(dt, input_manager, camera_manager);

		camera_manager.update(dt);

#ifdef _DEBUG
		if (current_engine_state == engine_state_editor) {
			entity_manager.is_paused = true;
			editor.update(dt, input_manager, camera_manager);
		}
		else {
			entity_manager.is_paused = false;
		}

		if (input_manager.key_down(SDL_SCANCODE_ESCAPE)) {
			if (current_engine_state == engine_state_gameplay)
				current_engine_state = engine_state_editor;
			else if (current_engine_state == engine_state_editor)
				current_engine_state = engine_state_gameplay;
		}
#endif // _DEBUG

		input_manager.update();

		/* ----- RENDER GAME ----- */
		renderer::start_drawing_frame();

		renderer::draw_models();

#ifdef _DEBUG
		if (current_engine_state == engine_state_editor) {
			editor.draw();
		}

		renderer::debug::draw_debug();
#endif // _DEBUG

		renderer::stop_drawing_frame();
	}

	/* ----- CLEAN GAME ----- */
	log_info("STARTING CLEANUP");

#ifdef _DEBUG
	editor.clean();
	renderer::debug::clean_debug();
#endif // _DEBUG

	renderer::clean();

	log_info("CLEANUP FINISHED");

	return 0;
}
