#pragma once
#include <stdio.h>
#include <fstream>
#include <glm.hpp>

#include "renderer.h"
#include "collision.h"

// TODO: Implement UUID System.
static uint32_t currentID = 0;

struct entity {
	uint32_t id;

	entity() : id(currentID++) {}
	~entity() {}

	virtual void update(double dt) {}
};

//
// ENTITIES
//

struct obstacle_entity {
	collision::box_collider* col;

	glm::vec2 pos;
	renderer::sprite* spr;

	obstacle_entity() {
		spr = renderer::create_sprite();
		spr->position = &pos;
		spr->layer = -1;
		spr->set_sprite_path("data/textures/face.png");

		pos = { 0,0 };

		col = collision::create_collider(spr->size);

		printf("OBSTACLE %p INITIALIZED\n", this);
	}

	~obstacle_entity() {
		renderer::delete_sprite(spr);
		collision::delete_collider(col);

		printf("OBSTACLE %p CLEANED\n", this);
	}

	void update(double dt) {
		col->pos = pos;
	}
};

struct sprite_entity {
	glm::vec2 pos;
	renderer::sprite* spr;

	std::string sprite_path;

	sprite_entity() {
		pos = { 0,0 };
		sprite_path = "";
		
		spr = renderer::create_sprite();

		spr->position = &pos;
		spr->layer = 0;
		spr->set_sprite_path(sprite_path.c_str());

		printf("SPRITE_ENTITY %p INITIALIZED\n", this);
	}

	~sprite_entity() {
		renderer::delete_sprite(spr);

		printf("SPRITE_ENTITY %p CLEANED\n", this);
	}

	void update(double dt) {

	}
};

struct enemy_entity {
	glm::vec2 pos;
	renderer::sprite* spr;
	collision::box_collider* col;

	enemy_entity() {
		pos = { 0,0 };
		
		spr = renderer::create_sprite();
		spr->position = &pos;
		spr->layer = -1;
		spr->set_sprite_path("data/textures/player.png");

		col = collision::create_collider(spr->size);

		printf("ENEMY_ENTITY %p INITIALIZED\n", this);
	}

	~enemy_entity() {
		renderer::delete_sprite(spr);
		collision::delete_collider(col);

		printf("ENEMY_ENTITY %p CLEANED\n", this);
	}

	void update(double dt) {
		col->pos = pos;
	}
};