#pragma once
#include <memory>

#include "entity.h"
#include "renderer.h"
#include "common.h"

struct player_entity : public entity {
	renderer::Model_Entity model;

	glm::ivec2 direction;

	const float walk_speed = 0.003f;
	const float run_speed = 0.006f;

	float interp_speed;
	
	glm::vec3 cam_original_pos;
	const float camera_speed = 0.01f;
	glm::vec3 fp_look_rotation = { 0, 270, 0 };
	glm::vec3 fp_look_direction = { 0, 270, 0 };
	bool is_first_person = false;

	player_entity();
	~player_entity();
	void update(double dt) override;
};