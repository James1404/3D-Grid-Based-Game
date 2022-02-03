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

	/*
	framebuffer_t framebuffer(1280, 720, GL_R32I, GL_RED_INTEGER, GL_DEPTH24_STENCIL8);

	float quad_vertices[] =
	{
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quad_vao, quad_vbo;
	glGenVertexArrays(1, &quad_vao);
	glGenBuffers(1, &quad_vbo);
	glBindVertexArray(quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	auto quad_shader = renderer::asset_manager.load_shader_from_file("data/shaders/framebuffer.glsl");

	glUseProgram(quad_shader->id);

	glUniform1i(glGetUniformLocation(quad_shader->id, "screenTexture"), 0);
	*/

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
		//renderer::debug::clear_debug_list();
#endif // _DEBUG

		// calculate delta time;
		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();
		double dt = ((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

		entity_manager.update(dt, input_manager, camera_manager);

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

		camera_manager.update(dt);

		input_manager.update();

		/* ----- RENDER GAME ----- */
#ifdef _DEBUG
		editor.bind_framebuffer();
		//framebuffer.bind();
		//glEnable(GL_DEPTH_TEST);

		renderer::clear_screen();

		entity_manager.draw();

		editor.draw_primitives();
		//renderer::draw_models();
		//renderer::debug::draw_debug();

		editor.unbind_framebuffer();
		//framebuffer.unbind();
		//glDisable(GL_DEPTH_TEST);

		editor.draw_framebuffer();
		//glUseProgram(quad_shader->id);
		//glBindVertexArray(quad_vao);
		//glBindTexture(GL_TEXTURE_2D, framebuffer.color_texture.id);
		//glDrawArrays(GL_TRIANGLES, 0, 6);

		if (current_engine_state == engine_state_editor) {
			editor.draw(input_manager);
		}

		renderer::swap_screen_buffers();
#else
		renderer::clear_screen();

		renderer::draw_models();

		renderer::swap_screen_buffers();
#endif // _DEBUG
	}

	/* ----- CLEAN GAME ----- */
	log_info("STARTING CLEANUP");

#ifdef _DEBUG
	editor.clean();
	renderer::debug::clean_debug();
#endif // _DEBUG

	renderer::shutdown();

	log_info("CLEANUP FINISHED");

	return 0;
}
