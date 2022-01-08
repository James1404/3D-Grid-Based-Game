#pragma once
#include "camera.h"
#include "entity.h"

struct editor {
	entity_manager* manager;

	float camera_zoom_speed = .001f;
	glm::vec2 velocity;
	float speed = 2;

	// TODO: make this a weak_ptr
	std::shared_ptr<entity> current_entity = nullptr;

	editor(entity_manager& _manager);
	~editor();

	void update(double dt);
	void draw();
};