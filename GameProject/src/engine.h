#include <functional>

#include "renderer.h"
#include "input.h"
#include "log.h"
#include "entity.h"
#include "editor.h"
#include "camera.h"

inline void engine_run(
	std::function<bool()> event_function,
	std::function<double()> delta_time_function)
{
	bool isRunning = false;

	entity_manager_t entity_manager;
	input_manager_t input_manager;
	camera_manager_t camera_manager;
#ifdef _DEBUG
	editor_manager editor;
#endif // _DEBUG

	enum class GAME_STATE {
		GAMEPLAY,
		EDITOR
	} static CurrentState;


	/* ----- INIT GAME ----- */
	renderer::init();
#ifdef _DEBUG
	renderer::debug::init_debug();
#endif // _DEBUG

	CurrentState = GAME_STATE::GAMEPLAY;

	// LEVEL NAMES :
	// combattestlevel
	// newtestlevel
	entity_manager.load("combattestlevel");

	input_manager.load();
#ifdef _DEBUG
	editor.init(entity_manager);
#endif // _DEBUG
	
	isRunning = true;

	while (isRunning) {
		/* ----- HANDLE EVENTS ----- */
		if(!event_function())
		{
			isRunning = false;
		}

		/* ----- UPDATE GAME ----- */
#ifdef _DEBUG
		renderer::debug::clear_debug_list();
#endif // _DEBUG

		// calculate delta time;
		double dt = delta_time_function();

		entity_manager.update(dt, input_manager, camera_manager);

		camera_manager.update(dt);

#ifdef _DEBUG
		if (CurrentState == GAME_STATE::EDITOR) {
			entity_manager.is_paused = true;
			editor.update(dt, input_manager, camera_manager);
		}
		else {
			entity_manager.is_paused = false;
		}

		if (input_manager.key_down(SDL_SCANCODE_ESCAPE)) {
			if (CurrentState == GAME_STATE::GAMEPLAY)
				CurrentState = GAME_STATE::EDITOR;
			else if (CurrentState == GAME_STATE::EDITOR)
				CurrentState = GAME_STATE::GAMEPLAY;
		}
#endif // _DEBUG

		input_manager.update();

		/* ----- RENDER GAME ----- */
		renderer::start_drawing_frame();
		
		renderer::draw_models();

#ifdef _DEBUG
		if (CurrentState == GAME_STATE::EDITOR) {
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
}
