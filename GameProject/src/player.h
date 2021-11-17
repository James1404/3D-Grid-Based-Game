#pragma once
#include <memory>

#include "entity.h"
#include "renderer.h"
#include "collision.h"

struct player_entity : public entity {
	renderer::sprite* spr;

	glm::vec2 target_pos;
	glm::vec2 vel;

	const int max_health_points = 3;
	int current_health_points;

	bool is_dead = false;

	player_entity();
	~player_entity();
	void update(double dt) override;

	void take_damage(int damage_amount);
};

extern std::shared_ptr<player_entity> player;
