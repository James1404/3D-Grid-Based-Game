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

		renderer_t::get().add_instance("data/models/block.gltf", "data/models/index.jpg", "data/shaders/model_loading.glsl", true, &visual_transform, index);
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
		renderer_t::get().add_instance("data/models/block.gltf", "data/models/image.jpg", "data/shaders/default.glsl", true, &visual_transform, index);
#endif
		visual_transform.scale = glm::vec3(0.1f);

		world_t::get().player.grid_pos = grid_pos;
		world_t::get().player.visual_transform.position = grid_pos;
	}
};

// TODO: Add room system and doors.
