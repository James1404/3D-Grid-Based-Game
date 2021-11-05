#pragma once
#include "entity.h"
#include "renderer.h"
#include "glm.hpp"

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <unordered_map>

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

	std::string trigger_event_name = "";

	std::vector<std::shared_ptr<enemy_entity>> enemies;
	std::vector<std::shared_ptr<obstacle_entity>> obstacles;
};

//
// EVENTS
//

struct listener {
	virtual ~listener() {}
	virtual void on_notify() = 0;
};

struct event_manager {
	std::unordered_multimap<std::string, listener*> events;

	void register_listener(std::string _event_name, listener* _listener);
	void remove_listener(std::string _event_name, listener* _listener);
	void notify(std::string _event_name);
	void clear();
};

//
// LISTENERS
//

struct cutscene : public listener {
	std::string event_name;

	cutscene();
	~cutscene();

	void update(double dt);

	void on_notify() override;

	bool is_active = false;
};

//
// LEVEL
//

struct level {
	std::string name;

	event_manager game_event_manager;

	std::vector<std::shared_ptr<cutscene>> cutscenes;
	std::vector<std::shared_ptr<path_node>> path_nodes;

	std::vector<std::shared_ptr<sprite_entity>> sprites;

	void init();
	void update(double dt);
	void clean();

	void save();
	void load(std::string level_name);
};

extern level current_level;
