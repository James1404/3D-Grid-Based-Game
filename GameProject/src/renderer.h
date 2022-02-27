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

namespace colour
{
	inline constexpr auto black = glm::vec3(0, 0, 0);
	inline constexpr auto white = glm::vec3(1, 1, 1);
	inline constexpr auto grey = glm::vec3(0.5f, 0.5f, 0.5f);
	inline constexpr auto red = glm::vec3(1, 0, 0);
	inline constexpr auto green = glm::vec3(0, 1, 0);
	inline constexpr auto blue = glm::vec3(0, 0, 1);
	inline constexpr auto yellow = glm::vec3(1, 1, 0);
	inline constexpr auto cyan = glm::vec3(0, 1, 1);
	inline constexpr auto purple = glm::vec3(1, 0, 1);
	inline constexpr auto pink = glm::vec3(1, 0.75f, 0.75f);
};

struct model_instance_data_t
{
	transform_t* transform;

	int index;
};

struct instance_container_t
{
	std::vector<model_instance_data_t> instance_data;

	std::shared_ptr<model_t> model;
	std::shared_ptr<texture_t> texture;
	std::shared_ptr<shader_t> shader;
	
	unsigned int instance_vbo;
	int instance_buffer_size;

	bool is_static;

	~instance_container_t();
	void construct_instance_buffers();
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

struct renderer_t
{
	SDL_GLContext context;

	glm::mat4 projection_matrix;
	glm::mat4 view_matrix;

	int screen_resolution_x = 1280, screen_resolution_y = 720;

	float near_clip_plane = 0.1f, far_clip_plane = 100.0f;

	primitive_renderer_t primitive_renderer;

	void init();
	void shutdown();

	void clear_screen();
	void swap_screen_buffers();

	std::vector<instance_container_t> instance_containers;

	void add_instance(std::string model_path, std::string texture_path, std::string shader_path, bool is_static, transform_t* transform, int index);

	void construct_all_instance_buffers();

	void update_all_instance_buffers();
	void update_non_static_instance_buffers();

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
