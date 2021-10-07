#pragma once
#include "entity.h"

#include "renderer.h"

#include <vector>
#include <memory>

struct level {
	std::string name;

	std::vector<std::shared_ptr<obstacle_entity>> obstacles;
	std::vector<std::shared_ptr<sprite_entity>> sprites;
	std::vector<std::shared_ptr<enemy_entity>> enemies;
	std::vector<glm::vec2> path_nodes;

	void init();
	void update(double dt);
	void clean();

	void save();
	void load(std::string level_name);
};

extern level runtime_level;