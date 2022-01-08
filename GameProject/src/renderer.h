#pragma once
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <string>

namespace colour {
	static const glm::vec3 black = glm::vec3(0, 0, 0);
	static const glm::vec3 white = glm::vec3(1, 1, 1);
	static const glm::vec3 grey = glm::vec3(0.5f, 0.5f, 0.5f);
	static const glm::vec3 red = glm::vec3(1, 0, 0);
	static const glm::vec3 green = glm::vec3(0, 1, 0);
	static const glm::vec3 blue = glm::vec3(0, 0, 1);
	static const glm::vec3 yellow = glm::vec3(1, 1, 0);
	static const glm::vec3 cyan = glm::vec3(0, 1, 1);
	static const glm::vec3 purple = glm::vec3(1, 0, 1);
	static const glm::vec3 pink = glm::vec3(1, 0.75f, 0.75f);
};

namespace renderer {
	const int cell_size = 16;

	extern SDL_Window* window;
	extern SDL_GLContext context;

	extern glm::mat4 projection;
	extern glm::mat4 view;

	extern int screen_resolution_x, screen_resolution_y;

	void init();
	void clean();

	void start_draw();
	void draw_sprites();
	void stop_draw();

	// TODO: Add Sprite Sheet
	// TODO: Batch Rendering

	struct renderable {
		virtual void draw() {}
		virtual ~renderable() {}

		glm::vec3* position;

		glm::vec3 colour;
		unsigned int vao;
	};

	struct sprite : public renderable {
		sprite();
		~sprite();

		void draw() override;
	};

	struct cube : public renderable {
		cube();
		~cube();

		void draw() override;
	};

#ifdef _DEBUG
	namespace debug {
		struct debug_drawing {
			unsigned int vao, vbo, ebo;

			bool screen_space;
			glm::vec3 colour;

			debug_drawing();
			~debug_drawing();
		};

		void init_debug();
		void clean_debug();

		void clear_debug_list();
		void draw_debug();

		void draw_line(const glm::vec2 p1, const glm::vec2 p2, const glm::vec3 colour, bool screen_space = false);
		void draw_box(const glm::vec2 position, const glm::vec2 size, const glm::vec3 colour, bool screen_space = false);
		void draw_box_wireframe(const glm::vec2 pos, const glm::vec2 size, const glm::vec3 colour, bool screen_space = false);
		void draw_circle(const glm::vec2 position, const float radius, const glm::vec3 colour, bool screen_space = false);
	}
#endif // _DEBUG
}
