#pragma once
#include "entity.h"
#include "renderer.h"
#include "collision.h"

#include <imgui.h>

struct obstacle : public entity {
	glm::vec2 pos;
	renderer::sprite* spr;
	collision::box_collider* col;

	obstacle() {
		spr = renderer::create_sprite("data/textures/face.png", &pos, -1);
		col = collision::create_collider(spr->size);

		printf("OBSTACLE %i INITIALIZED\n", id);
	}

	~obstacle() {
		renderer::delete_sprite(spr);
		collision::delete_collider(col);

		printf("OBSTACLE %i CLEANED\n", id);
	}

	void update(double dt) override {
		col->pos = pos;
	}
};