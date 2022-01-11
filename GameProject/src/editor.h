#pragma once
#include "camera.h"
#include "entity.h"

struct editor_manager {
	entity_manager* manager;

	glm::ivec3 cursor_grid_pos{ 0 };

	// TODO: make this a weak_ptr
	std::vector<std::weak_ptr<entity>> selected_entities;
	bool is_grabbed = false;

	void move_cursor(glm::vec3 _vel);
	
	void init(entity_manager& _manager);
	void clean();

	void update(double dt);
	void draw();
};