#pragma once
#include "common.h"
#include "entity.h"
#include "pathfinding.h"

struct block_entity : public entity
{
	model_entity_t model;

	block_entity()
		: model("data/models/block.gltf", "data/models/diffuse.jpg", &visual_pos, &visual_rotation, &visual_scale)
	{
		name = "block";
	}

	~block_entity()
	{

	}

	void update(double dt) override
	{
#ifdef _DEBUG
		model.index = index;
#endif

		visual_pos = grid_pos;
	}
};
