#pragma once
#include <memory>

#include "entity.h"
#include "renderer.h"
#include "collision.h"
#include "common.h"

struct player_entity : public entity {
	renderer::sprite spr;

	glm::vec2 vel;
	glm::ivec2 direction;

	const int walk_speed = 6;
	const int run_speed = 4;
	
	const int shoot_range = 5;
	uint32_t shoot_end_time = 0;
	const uint32_t shoot_cooldown_duration = 1000;

	enum player_states {
		IDLE = 0,
		MOVE_UP,
		MOVE_DOWN,
		MOVE_LEFT,
		MOVE_RIGHT,
		ATTACK,
		SHOOT
	};

	common::Limited_Queue<player_states, 2> state_queue;

	player_entity();
	~player_entity();
	void update_input(double dt) override;
	void update_logic() override;
	void update_visuals(double dt) override;
};