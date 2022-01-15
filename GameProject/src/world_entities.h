#pragma once
#include "common.h"
#include "entity.h"
#include "pathfinding.h"

struct block_entity : public entity {
	renderer::Model_Entity model;

	block_entity()
		: model("data/models/block/block.obj", &visual_pos)
	{
		tag = "block";
	}

	~block_entity() {

	}

	void update(double dt) override {
		visual_pos = grid_pos;
	}
};