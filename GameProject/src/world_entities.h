#pragma once
#include "common.h"
#include "entity.h"
#include "pathfinding.h"

struct block_entity : public entity {
	renderer::cube cb;

	block_entity() {
		tag = "block";

		cb.position = &visual_pos;
		cb.colour = { 1,0,0 };
	}

	~block_entity() {

	}

	void update(double dt) override {
		visual_pos = grid_pos;
	}
};