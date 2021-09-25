#pragma once
#include "entity.h"
#include "obstacle.h"

#include "renderer.h"

#include <vector>
#include <memory>

namespace level {
	struct level_data {
		std::vector<std::shared_ptr<obstacle>> obstacles;
	};
	extern level_data data;

	void init();
	void update(double dt);
	void clean();

	void save();
	void load();
}