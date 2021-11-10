#pragma once
#include <memory>

#include "entity.h"
#include "renderer.h"
#include "collision.h"

struct player_entity : public entity {
	renderer::sprite* spr;
	collision::box_collider* col;

	int current_node = 0;

	const int max_health_points = 3;
	int current_health_points;

	const int max_ammo = 8;
	int current_ammo;

	enum {
		PLAYER_DIRECTION_LEFT,
		PLAYER_DIRECTION_RIGHT
	} player_direction;

	enum {
		PLAYER_STANCE_STANDING,
		PLAYER_STANCE_CROUCHED
	} player_stance;

	enum {
		PLAYER_IDLE,
		PLAYER_RUNNING,
		PLAYER_AIMING,
		PLAYER_DODGING,
		PLAYER_RELOADING,
		PLAYER_STAGGERED,
		PLAYER_DEAD
	} player_state;

	player_entity();
	~player_entity();
	void update(double dt) override;

	void take_damage(int damage_amount);
};

extern std::shared_ptr<player_entity> player;
