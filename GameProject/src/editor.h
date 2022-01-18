#pragma once
#include "camera.h"
#include "entity.h"

struct editor_manager {
	entity_manager* manager;

	glm::vec3 cursor_pos{ 0 };

	// TODO: make this a weak_ptr
	std::vector<std::weak_ptr<entity>> selected_entities;
	bool is_grabbed = false;

	enum editor_mode {
		placement_cam = 0,
		free_cam
	};

	editor_mode mode = placement_cam;
	const float cam_movement_speed = 0.01f;
	const float cam_rotation_speed = 0.03f;
	bool is_cam_control = false;

	void move_cursor(glm::ivec3 _vel);
	
	void placement_cam_mode_update(double dt, std::shared_ptr<camera> cam);
	void free_cam_mode_update(double dt, std::shared_ptr<camera> cam);

	void init(entity_manager& _manager);
	void clean();

	void update(double dt);
	void draw();
};