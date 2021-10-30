#pragma once
#include <glm.hpp>

#include "renderer.h"
#include "collision.h"

typedef uint32_t ENTITY_FLAGS;
enum ENTITY_FLAGS_ {
	ENTITY_NONE = 0,
	ENTITY_DISABLED = 1 << 0,
	ENTTIY_INVISIBLE = 1 << 1,
};
static void ENTITY_FLAG_SET(ENTITY_FLAGS* x, ENTITY_FLAGS_ mask) { *x |= mask; }
static void ENTITY_FLAG_CLEAR(ENTITY_FLAGS* x, ENTITY_FLAGS_ mask) { *x &= ~mask; }
static void ENTITY_FLAG_TOGGLE(ENTITY_FLAGS* x, ENTITY_FLAGS_ mask) { *x ^= mask; }

static uint32_t current_id = 0;
struct entity {
	uint32_t id;
	ENTITY_FLAGS flags = 0;

	glm::vec2 pos;

	entity() : id(current_id++), pos(0,0) {
		printf("INITIALIZED ENTITY %p\n", this);
	}

	virtual ~entity() {
		printf("DESTROYED ENTITY %p\n", this);
	}
	virtual void update(double dt) {}
};

struct obstacle_entity : public entity {
	renderer::sprite* spr;
	collision::box_collider* col;

	obstacle_entity();
	~obstacle_entity();
	virtual void update(double dt);
};

struct sprite_entity : public entity{
	renderer::sprite* spr;

	// std::string sprite_path;

	sprite_entity();
	~sprite_entity();
	virtual void update(double dt);
};

struct enemy_entity : public entity {
	renderer::sprite* spr;
	collision::box_collider* col;

	const int max_health_points = 3;
	int current_health_points;

	glm::vec2 vel;
	int current_node = 0;

	enum {
		ENEMY_DIRECTION_LEFT,
		ENEMY_DIRECTION_RIGHT
	} enemy_direction;

	enemy_entity();
	~enemy_entity();
	virtual void update(double dt);

	void take_damage(int damage_points);
};
