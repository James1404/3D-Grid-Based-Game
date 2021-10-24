#pragma once
#include <memory>

#include "entity.h"
#include "renderer.h"
#include "collision.h"

struct player_entity : public entity {
	renderer::sprite* spr;
	collision::box_collider* col;

	glm::vec2 vel = { 0,0 };
	float speed = 50;
	int current_node = 0;
	
	enum {
		PLAYER_DIRECTION_LEFT,
		PLAYER_DIRECTION_RIGHT
	} player_direction;
	
	enum {
		PLAYER_STANDING,
		PLAYER_CROUCHED,
		PLAYER_STAGGERED,
		PLAYER_RELOADING,
		PLAYER_DEAD
	} player_state;

	player_entity();
	~player_entity();
	virtual void update(double dt);
};

extern std::shared_ptr<player_entity> player;