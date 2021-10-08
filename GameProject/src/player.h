#pragma once
#include "entity.h"
#include "renderer.h"
#include "collision.h"

namespace player {
	struct player_data {
		glm::vec2 pos = { 0,0 };
		glm::vec2 vel = { 0,0 };
		renderer::sprite* spr = nullptr;
		collision::box_collider* col = nullptr;
		float speed = 50;
		int current_node = 0;
	};

	extern player_data data;

	void init();
	void update(double dt);
	void clean();
}