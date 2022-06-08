#pragma once
#include <cassert>
#include <string>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "world.h"
#include "asset.h"

struct editor_t
{
	std::vector<std::weak_ptr<entity_t>> selected_entities;
	glm::ivec3 start_select, end_select;

	std::vector<entity_data_t> copied_data_entities;
	std::vector<glm::ivec3> placed_entities;

	enum class editor_mode
	{
		placement_cam = 0,
		free_cam
	};

	editor_mode mode = editor_mode::free_cam;
	const float cam_movement_speed = 0.01f;
	const float cam_rotation_speed = 0.2f;
	bool is_cam_control = false;

	bool can_use_keyboard = true;
	bool can_use_mouse = true;

	int gizmo_type = -1;

	enum
	{
		GRID_EDITING,
		VISUAL_EDITING
	} transform_type = GRID_EDITING;

	bool placement_mode = false;
	glm::vec3 placement_position = {};

	std::string placement_type = "";

	void init();
	void shutdown();

	void handle_events();

	void update(double dt);
	void draw();

	// misc functions
	int read_framebuffer_pixel(int x, int y);
	float read_framebuffer_depth_pixel(int x, int y);

	void clear_selected_entities();
	void select_entity(std::weak_ptr<entity_t> _entity);
	void add_multiselect_entity(std::weak_ptr<entity_t> _entity);
	void draw_entity_data(std::weak_ptr<entity_t> _entity);
	void draw_gizmo_at_selected_entity();

	// framebuffer
	texture_t depth_texture_attachment;
	texture_t color_texture_attachment;
	texture_t id_texture_attachment;

	unsigned int framebuffer_quad_vao, framebuffer_quad_vbo;
	std::shared_ptr<shader_t> framebuffer_quad_shader;

	unsigned int framebuffer_id;

	void init_framebuffer();
	void shutdown_framebuffer();
	void bind_framebuffer();
	void unbind_framebuffer();
	void draw_framebuffer();

	static editor_t& get()
	{
		static editor_t* instance = NULL;
		if(instance == NULL)
		{
			instance = new editor_t;
		}
		assert(instance);
		return *instance;
	}
};
