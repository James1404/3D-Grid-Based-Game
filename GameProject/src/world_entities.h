#pragma once
#include "common.h"
#include "world.h"
#include "pathfinding.h"

struct block_entity : public entity
{
	model_entity_t model;

	void init() override
	{
		name = "block";
		model.define_model("data/models/block.gltf", "data/models/diffuse.jpg", &visual_pos, &visual_rot, &visual_scl);

#ifdef _DEBUG
		model.index = index;
#endif
	}

	void update(double dt) override
	{
		//visual_pos = grid_pos;
	}
};

// TODO: Add room system and doors.
