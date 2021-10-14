#pragma once
#include "entity.h"

#include "renderer.h"

#include <vector>
#include <memory>

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
	glm::vec2 pos = { 0,0 };
	PATH_NODE_FLAGS flags = 0;
};

namespace level {
	struct level_data {
		std::string name;

		std::vector<std::shared_ptr<obstacle_entity>> obstacles;
		std::vector<std::shared_ptr<sprite_entity>> sprites;
		std::vector<std::shared_ptr<enemy_entity>> enemies;
		std::vector<std::shared_ptr<path_node>> path_nodes;
	};

	extern level_data data;

	void init();
	void update(double dt);
	void clean();

	void save();
	void load(std::string level_name);
}
