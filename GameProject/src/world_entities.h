#pragma once
#include "common.h"
#include "chunk.h"
#include "world.h"
#include "pathfinding.h"

struct block_entity : public entity_t
{
	void init() override
	{
		name = "block";
		model.define_model("data/models/block.gltf", "data/models/diffuse.jpg", &visual_transform);

#ifdef _DEBUG
		model.index = index;
#endif
	}

	void update(double dt) override
	{
		//visual_pos = grid_pos;
	}
};

struct player_spawn_entity : public entity_t
{
	void init() override
	{
		name = "player_spawn";

		flags.set(entity_flags_no_collision);

#ifdef _DEBUG
		model.define_model("data/models/block.gltf", "data/models/diffuse.jpg", &visual_transform);
		model.index = index;
#endif
		visual_transform.scale = glm::vec3(0.1f);

		world_t::get().player.grid_pos = grid_pos;
		world_t::get().player.visual_transform.position = grid_pos;
	}
};

// TODO: Add room system and doors.
