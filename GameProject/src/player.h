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

	player_entity();
	~player_entity();
	void update(double dt) override;
};