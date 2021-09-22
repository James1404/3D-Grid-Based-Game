#pragma once
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

namespace renderer {
	extern SDL_Window* window;
	extern SDL_GLContext context;

	extern glm::mat4 projection;
	extern glm::mat4 view;

	extern int screen_width, screen_height;
	extern const int screen_resolution_x, screen_resolution_y;

	void init();
	void clean();

	void start_draw();
	void draw_sprites();
	void stop_draw();

	// TODO: Add Sprite Sheet
	// TODO: Batch Rendering

	struct sprite {
		sprite(const char* _path, glm::vec2* _position, int _layer);
		~sprite();

		void draw();

		glm::vec2* position;
		int layer;
		int width, height;

		unsigned int shader;
		unsigned int vao;
		unsigned int texture;
	};

	sprite* create_sprite(const char* _path, glm::vec2* _position, int _layer);
	void delete_sprite(sprite* _sprite);
}