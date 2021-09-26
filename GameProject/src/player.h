#pragma once
#include "entity.h"
#include "renderer.h"
#include "collision.h"

namespace player {
	struct path {
		std::vector<glm::vec2> nodes = { {0,0}, {200, 50}, {80, 100} };
	};

	extern path player_path;

	struct player_data {
		glm::vec2 pos = { 0,0 };
		glm::vec2 vel = { 0,0 };
		renderer::sprite* spr = nullptr;
		collision::box_collider* col = nullptr;
		float speed = 50;

		unsigned int current_node = 0;
	};

	extern player_data data;

	void init();
	void update(double dt);
	void clean();
}