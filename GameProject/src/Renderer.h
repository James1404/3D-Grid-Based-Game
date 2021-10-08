#pragma once
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <string>

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
		sprite();
		~sprite();

		void draw();

		// void set_sprite_path(std::string sprite_name);

		glm::vec2* position;
		glm::ivec2 size;
		int layer;

		glm::vec3 colour;
		unsigned int vao;

		// lets keep things squares for now
		// unsigned int texture;
	};

	sprite* create_sprite();
	void delete_sprite(sprite* _sprite);

#ifdef _DEBUG
	namespace debug {
		void draw_square(const glm::vec2 position, const glm::vec2 size, const glm::vec3 colour);
	}
#endif // _DEBUG
}