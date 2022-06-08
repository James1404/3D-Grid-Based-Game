#pragma once
#include <memory>

#include "chunk.h"
#include "renderer.h"
#include "common.h"

/*
struct player_entity : public entity_t
{
	glm::vec3 vel;

	float walk_speed = 0.003f;
	float run_speed = 0.006f;

	float interp_speed;
	
	glm::vec3 cam_original_pos;
	float camera_speed = 0.01f;
	glm::vec3 fp_look_rotation = { 0, 270, 0 };
	glm::vec3 fp_look_direction = { 0, 270, 0 };
	bool is_first_person = false;

	enum
	{
		camera_view_default,
		camera_view_wall_infront,
	} camera_view_type = camera_view_default;

	void move_grid_pos(glm::ivec3 _dir);
	void init() override;
	void update(double dt) override;
};
*/

struct player_entity
{
	glm::ivec3 grid_pos = glm::ivec3(0), previous_grid_pos = glm::ivec3(0);
	transform_t visual_transform;

	glm::vec3 vel;

	float walk_speed = 0.003f;
	float run_speed = 0.006f;

	float interp_speed;
	
	glm::vec3 cam_original_pos;
	float camera_speed = 0.004f;
	glm::vec3 fp_look_rotation = { 0, 270, 0 };
	glm::vec3 fp_look_direction = { 0, 270, 0 };
	bool is_first_person = false;

	enum
	{
		camera_view_default,
		camera_view_wall_infront,
	} camera_view_type = camera_view_default;

	void init();
	void update(double dt);

	void move_grid_pos(glm::ivec3 _dir);
	bool is_grounded(glm::ivec3 _pos);
};
