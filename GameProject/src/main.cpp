#include <SDL.h>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>
#include <winpackagefamily.h>

#include "renderer.h"
#include "window.h"
#include "input.h"
#include "log.h"
#include "editor.h"
#include "camera.h"
#include "event.h"
#include "world.h"

enum engine_state
{
	engine_state_gameplay,
	engine_state_editor
};

int main(int argc, char* args[])
{
	init_window("Game", 1280, 720);

	world_t game_world;
	event_manager_t event_manager;

	engine_state current_engine_state;

	init_renderer();

	init_input();
	load_input();
	
	// LEVEL NAMES :
	// combattestlevel
	game_world.load();

	current_engine_state = engine_state_gameplay;

#ifdef _DEBUG
	init_editor(game_world);
#endif // _DEBUG

	uint64_t NOW = SDL_GetPerformanceCounter(), LAST = 0;
	while (window_is_running)
	{
		/* ----- HANDLE EVENTS ----- */
		if (handle_window_events())
		{
#ifdef _DEBUG
			handle_editor_events();
#endif // _DEBUG
		}

		/* ----- UPDATE GAME ----- */
		// calculate delta time;
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		double dt = ((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

		game_world.update(dt);

#ifdef _DEBUG
		if (current_engine_state == engine_state_editor)
		{
			game_world.is_paused = true;
			update_editor(dt);
		}
		else
		{
			game_world.is_paused = false;
		}

		if (input_key_down(SDL_SCANCODE_ESCAPE)) {
			if (current_engine_state == engine_state_gameplay)
				current_engine_state = engine_state_editor;
			else if (current_engine_state == engine_state_editor)
				current_engine_state = engine_state_gameplay;
		}
#endif // _DEBUG

		update_camera();
		update_input();

		/* ----- RENDER GAME ----- */
#ifdef _DEBUG
		bind_editor_framebuffer();

		renderer_clear_screen();

		renderer_draw();

		unbind_editor_framebuffer();

		draw_editor_framebuffer();

		if (current_engine_state == engine_state_editor)
		{
			draw_editor();
		}

		renderer_swap_screen_buffers();
#else
		renderer_clear_screen();

		renderer_draw();

		renderer_swap_screen_buffers();
#endif // _DEBUG
	}

	/* ----- CLEAN GAME ----- */
	log_info("STARTING CLEANUP");

#ifdef _DEBUG
	shutdown_editor();
#endif // _DEBUG

	game_world.shutdown();

	shutdown_input();

	shutdown_renderer();
	shutdown_window();

	log_info("CLEANUP FINISHED");

	return 0;
}
