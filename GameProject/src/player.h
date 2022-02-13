#pragma once
#include <memory>

#include "world.h"
#include "renderer.h"
#include "common.h"

struct player_entity : public entity
{
	model_entity_t model;

	glm::vec3 vel;

	float walk_speed = 0.003f;
	float run_speed = 0.006f;

	float interp_speed;
	
	glm::vec3 cam_original_pos;
	float camera_speed = 0.01f;
	glm::vec3 fp_look_rotation = { 0, 270, 0 };
	glm::vec3 fp_look_direction = { 0, 270, 0 };
	bool is_first_person = false;

	void init() override;
	void update(double dt) override;
};
