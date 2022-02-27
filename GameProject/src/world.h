#pragma once
#include <vector>
#include <memory>
#include <cassert>
#include <typeindex>
#include <string>

#include "chunk.h"

#include "player.h"

struct world_t
{
	bool is_paused = false;

	std::vector<std::shared_ptr<chunk_t>> chunks;
	int current_chunk = 0;

	void init();
	void shutdown();

	std::shared_ptr<chunk_t> get_current_chunk();
	player_entity player;

	void clear_world_data();
	void update(double dt);

	void save();
	void load();

	int new_chunk();
	void delete_chunk(int index);

	static world_t& get()
	{
		static world_t* instance = NULL;
		if(instance == NULL)
		{
			instance = new world_t;
		}
		assert(instance);
		return *instance;
	}
};

template<typename T>
std::shared_ptr<T> new_entity();

bool create_entity_by_type(entity_data_t& data, std::shared_ptr<entity_t>& entity);
void add_entity(std::shared_ptr<chunk_t> chunk, entity_data_t data);
void add_entity_and_init(std::shared_ptr<chunk_t> chunk, entity_data_t data);
void remove_entity(std::shared_ptr<chunk_t> chunk, std::weak_ptr<entity_t> _entity);

void transition_chunk(std::shared_ptr<chunk_t> next_chunk, entity_t& entity);
