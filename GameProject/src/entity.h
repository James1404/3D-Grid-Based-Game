#pragma once
#include <stdio.h>
#include <fstream>
#include <glm.hpp>
#include <string>
#include <vector>
#include <algorithm>

#include "renderer.h"
#include "collision.h"
#include "player.h"

static uint32_t current_uuid = 0;
struct uuid {
	uint32_t id;

	uuid() : id(current_uuid++) {}
	~uuid() {}
};

//
// ENTITIES
//

struct obstacle_entity {
	uuid id;
	collision::box_collider* col;
	renderer::sprite* spr;

	glm::vec2 pos;

	obstacle_entity() {
		pos = { 0,0 };

		spr = renderer::create_sprite();
		spr->position = &pos;
		spr->layer = -1;
		//spr->set_sprite_path("face.png");
		spr->size = { 48,48 };
		spr->colour = { 0,1,0 };


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
	uuid id;
	renderer::sprite* spr;

	glm::vec2 pos;
	// std::string sprite_path;

	sprite_entity() {
		pos = { 0,0 };
		// sprite_path = "";
		
		spr = renderer::create_sprite();

		spr->position = &pos;
		spr->layer = 0;
		//spr->set_sprite_path(sprite_path.c_str());
		spr->size = { 1,1 };
		spr->colour = { 0,0,0 };

		printf("SPRITE_ENTITY %p INITIALIZED\n", this);
	}

	~sprite_entity() {
		renderer::delete_sprite(spr);

		printf("SPRITE_ENTITY %p CLEANED\n", this);
	}

	void update(double dt) { }
};

struct enemy_entity { 
	uuid id;	
	renderer::sprite* spr;
	collision::box_collider* col;

	glm::vec2 pos;

	enemy_entity() {
		pos = { 0,0 };

		spr = renderer::create_sprite();
		spr->position = &pos;
		spr->layer = -1;
		//spr->set_sprite_path("player.png");
		spr->size = { 16,80 };
		spr->colour = { 1,0,0 };

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

typedef uint32_t PATH_NODE_FLAGS;
enum PATH_NODE_FLAGS_ {
	PATH_NODE_NONE		= 0,
	PATH_NODE_COMBAT	= 1 << 0,
	PATH_NODE_SLOW		= 1 << 1,
	PATH_NODE_FAST		= 1 << 2
};

static void PATH_NODE_FLAG_SET(uint32_t* x, PATH_NODE_FLAGS_ mask) { *x |= mask; }
static void PATH_NODE_FLAG_CLEAR(uint32_t* x, PATH_NODE_FLAGS_ mask) { *x &= ~mask; }
static void PATH_NODE_FLAG_TOGGLE(uint32_t* x, PATH_NODE_FLAGS_ mask) { *x ^= mask; }

struct path_node {
	uuid id;
	glm::vec2 pos = { 0,0 };
	PATH_NODE_FLAGS flags = 0;
};

//
// EVENTS
//

typedef void (*callback_function)(void);
struct game_event {
	std::vector<callback_function> registered_functions;
	
	std::string event_name;

	void register_function(callback_function func) {
		registered_functions.push_back(*func);
	}

	void remove_function(callback_function func) {
		registered_functions.erase(std::remove(registered_functions.begin(), registered_functions.end(), func), registered_functions.end());
	}

	void notify() {
		for(auto& func : registered_functions) {
			func();
		}
	}
};

struct trigger_entity {
	uuid id;

	collision::box_collider* col;

	glm::vec2 pos;
	glm::ivec2 size;

	game_event* linked_event;

	trigger_entity() {
		pos = { 0,0 };
		size = { 0,0 };
		col = collision::create_collider(size);

		printf("TRIGGER_ENTITY %p INITIALIZED\n", this);
	}

	~trigger_entity() {
		collision::delete_collider(col);
		printf("TRIGGER_ENTITY %p CLEANED\n", this);
	}

	void update(double dt) {
		col->pos = pos;
		col->size = size;

		if (collision::check_box_collision(col)) {
			linked_event->notify();
		}
	}
};

struct cutscene_entity {
	uuid id;

	game_event* linked_event;

	// TODO: uncomment register and remove function
	cutscene_entity() {
		//linked_event->register_function(start_cutscene);
		printf("CUTSCENE_ENTITY %p INITIALIZED\n", this);
	}

	~cutscene_entity() {
		//linked_event->remove_function(start_cutscene);
		printf("CUTSCENE_ENTITY %p CLEANED\n", this);
	}

	static void start_cutscene() {
		printf("Cutscene started\n");
	}
};
