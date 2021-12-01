#pragma once
#include <memory>

#include "entity.h"
#include "renderer.h"
#include "collision.h"

struct player_entity : public entity {
	renderer::sprite* spr;

	glm::ivec2 previous_pos;
	glm::vec2 target_pos;
	glm::vec2 vel;

	glm::ivec2 direction;

	const int shoot_range = 5;
	uint32_t shoot_end_time = 0;
	const uint32_t shoot_cooldown_duration = 1000;

	const float movement_speed = 0.005f;

	player_entity();
	~player_entity();
	void update(double dt) override;
};