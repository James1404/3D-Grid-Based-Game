#pragma once
#include "entity.h"
#include "renderer.h"
#include "collision.h"

struct player_entity : public entity {
	glm::vec2 vel = { 0,0 };
	float speed = 50;
	int current_node = 0;

	player_entity();
	virtual void update(double dt);
};