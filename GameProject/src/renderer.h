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
#include <map>
#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"
#include <unordered_map>

#include "log.h"

struct model_instance_data_t
{
	transform_t* transform;

#ifdef _DEBUG
	int index;
#endif
	void define_model(glm::vec3* _position, glm::vec3* _rotation, glm::vec3* _scale);
};

struct model_entity_t
{
	std::shared_ptr<model_t> model;
	std::shared_ptr<shader_t> shader;
	std::shared_ptr<texture_t> texture;

	transform_t* transform;

	bool is_paused;

#ifdef _DEBUG
	int index;
#endif

	model_entity_t();
	~model_entity_t();

	void define_model(std::string _model_path, std::string _texture_path, transform_t* _transform);
	void draw();
};

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

struct primitive_renderer_t
{
	std::shared_ptr<shader_t> line_shader, quad_shader;
	unsigned int line_vao, line_vbo, line_ebo;
	unsigned int quad_vao, quad_vbo, quad_ebo;

	void add_line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 colour);
	void add_wireframe_cube(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour);
	void add_quad(const glm::vec3 pos, const glm::vec3 rot, const glm::vec2 scl, const glm::vec3 colour);

	void draw_line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 colour);
	void draw_box_wireframe(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour);
	void draw_quad(const glm::vec3 pos, const glm::vec3 rot, const glm::vec2 scl, const glm::vec3 colour);

	void draw();
	void init();
	void shutdown();
};

/*
namespace std
{
	template<> struct less<texture_t>
	{
		bool operator() (const texture_t& lhs, const texture_t& rhs)
		{
			return lhs.id < rhs.id;
		}
	};

	template<> struct less<model_t>
	{
		bool operator() (const model_t& lhs, const model_t& rhs)
		{
			return lhs.id < rhs.id;
		}
	};
}
*/

struct renderer_t
{
	SDL_GLContext context;

	glm::mat4 projection_matrix;
	glm::mat4 view_matrix;

	int screen_resolution_x = 1280, screen_resolution_y = 720;

	float near_clip_plane = 0.1f, far_clip_plane = 100.0f;

	primitive_renderer_t primitive_renderer;

	std::vector<model_entity_t*> model_list;
	//std::map<texture_t, std::map<model_t, std::vector<model_entity_t>>> unique_draw_calls;

	void init();
	void shutdown();

	void clear_screen();
	void swap_screen_buffers();

	void draw();

	void set_resolution(int x, int y);

	static renderer_t& get()
	{
		static renderer_t* instance = NULL;
		if(instance == NULL)
		{
			instance = new renderer_t;
		}
		assert(instance);
		return *instance;
	}
};
