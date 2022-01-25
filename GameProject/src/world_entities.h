#pragma once
#include "common.h"
#include "entity.h"
#include "pathfinding.h"

struct block_entity : public entity {
	renderer::model_entity_t model;

	block_entity()
		: model("data/models/block.gltf", "data/models/diffuse.jpg", &visual_pos)
	{
		name = "block";
	}

	~block_entity() {

	}

	void update(double dt, input_manager_t& input_manager, camera_manager_t& camera_manager) override
	{
		visual_pos = grid_pos;
	}
};
