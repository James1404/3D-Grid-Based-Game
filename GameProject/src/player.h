#pragma once
#include <memory>

#include "entity.h"
#include "renderer.h"
#include "collision.h"

struct player_entity : public entity {
	renderer::sprite spr;

	glm::vec2 vel;
	glm::ivec2 direction;

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
	} player_state;

	player_entity();
	~player_entity();
	void update_input(double dt) override;
	void update_logic(int steps) override;
	void update_visuals(double dt) override;
};