#pragma once
#include "entity.h"
#include "renderer.h"

#include <vector>
#include <memory>
#include <string>

namespace level {
	struct level_data {
		std::string name;

		std::vector<std::shared_ptr<obstacle_entity>> obstacles;
		std::vector<std::shared_ptr<sprite_entity>> sprites;
		std::vector<std::shared_ptr<enemy_entity>> enemies;
		std::vector<std::shared_ptr<path_node>> path_nodes;
		std::vector<std::shared_ptr<trigger_entity>> triggers;
		std::vector<std::shared_ptr<cutscene_entity>> cutscenes;
		std::vector<std::shared_ptr<game_event>> game_events;
	};

	extern level_data data;

	void init();
	void update(double dt);
	void clean();

	void save();
	void load(std::string level_name);
}
