#pragma once
#include "camera.h"
#include "entity.h"
#include "input.h"

struct editor_manager {
	entity_manager_t* entity_manager;

	glm::vec3 cursor_pos{ 0 };

	// TODO: make this a weak_ptr
	std::vector<std::weak_ptr<entity>> selected_entities;
	bool is_grabbed = false;

	enum class editor_mode {
		placement_cam = 0,
		free_cam
	};

	editor_mode mode = editor_mode::placement_cam;
	const float cam_movement_speed = 0.01f;
	const float cam_rotation_speed = 0.03f;
	bool is_cam_control = false;

	bool can_use_keyboard = true;
	bool can_use_mouse = true;

	void move_cursor(glm::ivec3 _vel);

	void select_entity(std::weak_ptr<entity> _entity);
	void add_multiselect_entity(std::weak_ptr<entity> _entity);
	void clear_selected_entities();

	void placement_cam_mode_update(double dt, input_manager_t& input_manager, std::shared_ptr<camera_t> cam);
	void free_cam_mode_update(double dt, input_manager_t& input_manager, std::shared_ptr<camera_t> cam);

	void init(entity_manager_t& _manager);
	void clean();

	void update(double dt, input_manager_t& input_manager, camera_manager_t& camera_manager);
	void draw();
};
