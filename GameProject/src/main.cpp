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
	engine_state current_engine_state = engine_state_gameplay;

	window_t::get().init("Game", 1280, 720);

	asset_manager_t::get().init();

	renderer_t::get().init();

	input_t::get().init();
	input_t::get().load();
	
	world_t::get().load();

	event_manager_t::get().init();

#ifdef _DEBUG
	editor_t::get().init();
#endif // _DEBUG

	uint64_t NOW = SDL_GetPerformanceCounter(), LAST = 0;
	while (window_t::get().window_is_running)
	{
		/* ----- HANDLE EVENTS ----- */
		if (window_t::get().handle_events())
		{
#ifdef _DEBUG
			editor_t::get().handle_events();
#endif // _DEBUG
		}

		/* ----- UPDATE GAME ----- */
		// calculate delta time;
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		double dt = ((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

		world_t::get().update(dt);

#ifdef _DEBUG
		if (current_engine_state == engine_state_editor)
		{
			world_t::get().is_paused = true;
			editor_t::get().update(dt);
		}
		else
		{
			world_t::get().is_paused = false;
		}

		if (input_t::get().key_down(SDL_SCANCODE_ESCAPE)) {
			if (current_engine_state == engine_state_gameplay)
			{
				world_t::get().load();
				current_engine_state = engine_state_editor;
			}
			else if (current_engine_state == engine_state_editor)
			{
				current_engine_state = engine_state_gameplay;
			}
		}
#endif // _DEBUG

		camera_manager_t::get().update();
		input_t::get().update();

		/* ----- RENDER GAME ----- */
#ifdef _DEBUG
		editor_t::get().bind_framebuffer();

		renderer_t::get().clear_screen();

		renderer_t::get().draw();

		editor_t::get().unbind_framebuffer();

		editor_t::get().draw_framebuffer();

		if (current_engine_state == engine_state_editor)
		{
			editor_t::get().draw();
		}

		renderer_t::get().swap_screen_buffers();
#else
		renderer_t::get().clear_screen();

		renderer_t::get().draw();

		renderer_t::get().swap_screen_buffers();
#endif // _DEBUG
	}

	/* ----- CLEAN GAME ----- */
	log_info("STARTING CLEANUP");
	
#ifdef _DEBUG
	editor_t::get().shutdown();
#endif // _DEBUG

	camera_manager_t::get().shutdown();

	event_manager_t::get().shutdown();
	world_t::get().shutdown();

	input_t::get().shutdown();

	renderer_t::get().shutdown();
	asset_manager_t::get().shutdown();
	window_t::get().shutdown();

	log_info("CLEANUP FINISHED");

	return 0;
}
