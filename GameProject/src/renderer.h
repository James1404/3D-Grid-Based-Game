#pragma once
#include <SDL.h>
#include <GL/glew.h>
#include <GL/GLU.h>
#include "asset.h"
#include <SDL_opengl.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"

#include "log.h"

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

extern SDL_GLContext context;

extern glm::mat4 projection_matrix;
extern glm::mat4 view_matrix;

extern int screen_resolution_x, screen_resolution_y;

void init_renderer();
void shutdown_renderer();

void renderer_clear_screen();
void renderer_swap_screen_buffers();

void renderer_draw();

struct model_entity_t {
	std::shared_ptr<model_t> model;
	std::shared_ptr<shader_t> shader;
	std::shared_ptr<texture_t> texture;

	glm::vec3* position;
	glm::vec3* rotation;
	glm::vec3* scale;

	bool is_paused;

#ifdef _DEBUG
	int index;
#endif
	model_entity_t(std::string _model_path, std::string _texture_path, glm::vec3* _position, glm::vec3* _rotation, glm::vec3* _scale);
	~model_entity_t();

	void draw();
};

void add_primitive_line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 colour);
void add_primitive_wireframe_cube(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour);
void draw_primitives();
void init_primitives();
void shutdown_primitives();
