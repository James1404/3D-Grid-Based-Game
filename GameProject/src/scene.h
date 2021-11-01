#pragma once
#include "entity.h"
#include "renderer.h"
#include "glm.hpp"

#include <vector>
#include <memory>
#include <string>

//
// PATH
//

typedef uint32_t PATH_NODE_FLAGS;
enum PATH_NODE_FLAGS_ {
	PATH_NODE_NONE		= 0,
	PATH_NODE_SLOW		= 1 << 0,
	PATH_NODE_FAST		= 1 << 1
};
static void PATH_NODE_FLAG_SET(PATH_NODE_FLAGS* x, PATH_NODE_FLAGS_ mask) { *x |= mask; }
static void PATH_NODE_FLAG_CLEAR(PATH_NODE_FLAGS* x, PATH_NODE_FLAGS_ mask) { *x &= ~mask; }
static void PATH_NODE_FLAG_TOGGLE(PATH_NODE_FLAGS* x, PATH_NODE_FLAGS_ mask) { *x ^= mask; }

struct path_node {
	glm::vec2 pos = { 0,0 };
	PATH_NODE_FLAGS flags = 0;

	bool is_trigger = false;
	std::string trigger_event_name = "";

	std::vector<std::shared_ptr<enemy_entity>> enemies;
	std::vector<std::shared_ptr<obstacle_entity>> obstacles;
};

//
// EVENTS
//

typedef void (*callback_function)(void);
struct game_event {
	std::vector<callback_function> registered_functions;
	
	std::string event_name;

	void register_function(callback_function func);
	void remove_function(callback_function func);
	void notify();
};

static bool find_game_event(game_event* _event, std::string _name);

//
// CUTSCENE
//

struct cutscene {
	void init();
	void clean();

	static void start_cutscene();
	void update(double dt);
};

//
// LEVEL
//

struct level {
	std::string name;

	std::vector<std::shared_ptr<entity>> entities;

	std::vector<std::shared_ptr<cutscene>> cutscenes;
	std::vector<std::shared_ptr<game_event>> game_events;
	std::vector<std::shared_ptr<path_node>> path_nodes;

	std::vector<std::shared_ptr<sprite_entity>> sprites;

	void init();
	void update(double dt);
	void clean();

	void save();
	void load(std::string level_name);
};

extern level current_level;
