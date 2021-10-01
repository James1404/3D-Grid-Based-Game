#pragma once
#include "entity.h"
#include "obstacle.h"

#include "renderer.h"

#include <vector>
#include <memory>

namespace level {
	struct level_data {
		std::vector<std::shared_ptr<obstacle>> obstacles;
		std::vector<renderer::sprite*> sprites;
		std::vector<glm::vec2> path_nodes;
	};
	extern level_data data;

	void init();
	void update(double dt);
	void clean();

	void save(std::string level_name);
	void load(std::string level_name);
}