#pragma once
#include <stdio.h>
#include <fstream>
#include <glm.hpp>
#include <string>
#include <vector>
#include <algorithm>

#include "renderer.h"
#include "collision.h"

//
// ENTITIES
//

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
	renderer::sprite* spr;
	collision::box_collider* col;

	entity() : id(current_id++), pos(0,0) {
		spr = renderer::create_sprite();
		spr->position = &pos;
		spr->layer = 0;
		spr->size = { 20,20 };
		spr->colour = colour::green;
		
		col = collision::create_collider();

		printf("INITIALIZED ENTITY %p", this);
	}

	virtual ~entity() {
		renderer::delete_sprite(spr);
		collision::delete_collider(col);
		
		printf("DESTROYED ENTITY %p", this);
	}
	virtual void update(double dt) {}
};

struct obstacle_entity : public entity {
	obstacle_entity() {
		spr->layer = -1;
		spr->size = { 48,48 };
		spr->colour = colour::green;

		col->size = spr->size;
	}

	virtual void update(double dt) {
		col->pos = pos;
	}
};

struct sprite_entity : public entity{
	// std::string sprite_path;

	sprite_entity() {
		collision::delete_collider(col);
		// sprite_path = "";
		
		spr->layer = 0;
		//spr->set_sprite_path(sprite_path.c_str());
		spr->size = { 1,1 };
		spr->colour = { 0,0,0 };
	}

	virtual void update(double dt) { }
};

struct enemy_entity : public entity { 
	enemy_entity() {
		spr->layer = -1;
		//spr->set_sprite_path("player.png");
		spr->size = { 16,80 };
		spr->colour = { 1,0,0 };

		col->size = spr->size;
	}

	virtual void update(double dt) {
		col->pos = pos;
	}
};
