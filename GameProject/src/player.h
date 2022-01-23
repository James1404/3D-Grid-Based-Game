#pragma once
#include <memory>

#include "entity.h"
#include "renderer.h"
#include "common.h"

struct player_entity : public entity {
	renderer::model_entity_t model;

	glm::vec3 vel;

	float walk_speed = 0.003f;
	float run_speed = 0.006f;

	float interp_speed;
	
	glm::vec3 cam_original_pos;
	float camera_speed = 0.01f;
	glm::vec3 fp_look_rotation = { 0, 270, 0 };
	glm::vec3 fp_look_direction = { 0, 270, 0 };
	bool is_first_person = false;

	player_entity();
	~player_entity();
	void update(double dt, input_manager_t& input_manager, camera_manager_t& camera_manager) override;
};
