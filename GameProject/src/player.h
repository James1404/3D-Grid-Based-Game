#pragma once
#include <memory>

#include "entity.h"
#include "renderer.h"
#include "collision.h"
#include "common.h"

struct player_entity : public entity {
	renderer::sprite spr;

	glm::ivec2 direction;

	const float walk_speed = 0.003f;
	const float run_speed = 0.006f;

	float interp_speed;
	
	const int shoot_range = 5;
	uint32_t shoot_end_time = 0;
	const uint32_t shoot_cooldown_duration = 1000;

	enum player_actions {
		IDLE = 0,
	};

	common::Limited_Queue<player_actions, 1> state_queue;

	glm::vec3 cam_original_pos;
	const float camera_speed = 0.01f;

	player_entity();
	~player_entity();
	void update(double dt) override;
};