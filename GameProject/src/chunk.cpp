#include "chunk.h"

#include "common.h"

//
// ENTITY FLAGS
//

entity_flags_t::entity_flags_t()
	: m_flags(0)
{
}

entity_flags_t::entity_flags_t(uint32_t flags)
	: m_flags(flags)
{
}

void entity_flags_t::set(entity_flags mask)
{
	m_flags |= mask;
}

void entity_flags_t::clear(entity_flags mask)
{
	m_flags &= ~mask;
}

void entity_flags_t::toggle(entity_flags mask)
{
	m_flags ^= mask;
}

bool entity_flags_t::has(entity_flags mask)
{
	return m_flags & mask;
}

//
// ENTITY
//

entity_data_t entity_t::extract_data()
{
	/*
	entity_data_t data;
	data.type = name;
	data.flags = flags;
	data.grid_pos = grid_pos;
	data.visual_transform.position = visual_transform.position;
	data.visual_transform.rotation = visual_transform.rotation;
	data.visual_transform.scale = visual_transform.scale;

	return data; 
	*/

	return entity_data_t(name, id, flags, grid_pos);
}

bool entity_t::is_grounded(glm::ivec3 _pos)
{
	return chunk->check_collisions(_pos + glm::ivec3(0, -1, 0), this);
}

bool entity_t::is_moving() const
{
	return (visual_transform.position != (glm::vec3)grid_pos);
}

void entity_t::set_grid_pos(glm::vec3 _pos)
{
	previous_grid_pos = grid_pos;
	grid_pos = _pos;
}

void entity_t::revert_grid_pos()
{
	grid_pos = previous_grid_pos;
}

void entity_t::interp_visuals(double dt, float interp_speed)
{
	// TODO: implement lerping instead of move_towards
	//visual_transform.position = lerp(visual_transform.position, (glm::vec3)grid_pos, interp_speed * dt);
	visual_transform.position = move_towards(visual_transform.position, grid_pos, interp_speed * dt);
}

//
// CHUNK
//

void chunk_t::clear_data()
{
	entities.clear();
}

std::vector<glm::ivec3> chunk_t::neighbors(glm::ivec3 _pos) const
{
	std::vector<glm::ivec3> results;

	std::vector<glm::ivec2> DIRS = { {0,1}, {0, -1}, {1,0}, {-1, 0} };
	for (auto dir : DIRS)
	{
		glm::ivec3 next = _pos + glm::ivec3(dir,0);
		
		results.push_back(next);
	}

	if ((_pos.x + _pos.y) % 2 == 0)
		std::reverse(results.begin(), results.end());

	return results;
}

std::vector<glm::ivec3> chunk_t::diagonal_neighbors(glm::ivec3 _pos) const
{
	std::vector<glm::ivec3> results;

	// TODO: change directions to 3D
	std::vector<glm::ivec2> DIRS = { {0,1}, {0, -1}, {1,0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
	for (auto dir : DIRS)
	{
		glm::ivec3 next = _pos + glm::ivec3(dir, 0);

		results.push_back(next);
	}

	if ((_pos.x + _pos.y) % 2 == 0)
		std::reverse(results.begin(), results.end());

	return results;
}

std::weak_ptr<entity_t> chunk_t::find_entity_by_id(uuid _id) const
{
	for (auto& entity : entities)
	{
		if (entity->id == _id)
		{
			return entity;
		}
	}

	//log_warning("ENTITY WITH ID DOES NOT EXIST");
	return std::weak_ptr<entity_t>();
}

std::weak_ptr<entity_t> chunk_t::find_entity_by_index(int _index) const
{
	for (auto& entity : entities)
	{
		if (entity->index == _index)
		{
			return entity;
		}
	}

	//log_warning("Entity with index does not exist");
	return std::weak_ptr<entity_t>();
}

std::weak_ptr<entity_t> chunk_t::find_entity_by_position(glm::vec3 _pos) const
{
	for (auto& entity : entities)
	{
		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return entity;
		}
	}

	log_warning("ENTITY AT POSITION DOES NOT EXIST");
}

bool chunk_t::is_entity_at_position(glm::vec3 _pos) const
{
	for (auto& entity : entities)
	{
		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return true;
		}
	}

	return false;
}

std::weak_ptr<entity_t> chunk_t::get_entity_at_position(glm::vec3 _pos)
{
	for (auto& entity : entities)
	{
		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return entity;
		}
	}

	return std::weak_ptr<entity_t>();
}

bool chunk_t::check_collisions(glm::vec3 _pos) const
{
	for(auto& entity : entities)
	{
		if (entity->flags.has(entity_flags_no_collision))
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return true;
		}
	}

	return false;
}

bool chunk_t::check_collisions(glm::vec3 _pos, entity_t* _ignored_entity) const
{
	for (auto& entity : entities)
	{
		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags.has(entity_flags_no_collision))
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return true;
		}
	}

	return false;
}

bool chunk_t::check_collisions(glm::vec3 _pos, std::string _tag) const
{
	for (auto& entity : entities)
	{
		if (entity->name != _tag)
			continue;

		if (entity->flags.has(entity_flags_no_collision))
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return true;
		}
	}

	return false;
}

bool chunk_t::check_collisions(glm::vec3 _pos, entity_t* _ignored_entity, std::string _tag) const {
	for (auto& entity : entities)
	{
		if (entity->name != _tag)
			continue;

		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags.has(entity_flags_no_collision))
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return true;
		}
	}

	return false;
}

std::weak_ptr<entity_t> chunk_t::get_collisions(glm::vec3 _pos)
{
	for (auto& entity : entities)
	{
		if (entity->flags.has(entity_flags_no_collision))
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return entity;
		}
	}

	return std::weak_ptr<entity_t>();
}

std::weak_ptr<entity_t> chunk_t::get_collisions(glm::vec3 _pos, entity_t* _ignored_entity)
{
	for (auto& entity : entities)
	{
		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags.has(entity_flags_no_collision))
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return entity;
		}
	}

	return std::weak_ptr<entity_t>();
}

std::weak_ptr<entity_t> chunk_t::get_collisions(glm::vec3 _pos, std::string _tag)
{
	for (auto& entity : entities)
	{
		if (entity->name != _tag)
			continue;

		if (entity->flags.has(entity_flags_no_collision))
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return entity;
		}
	}

	return std::weak_ptr<entity_t>();
}

std::weak_ptr<entity_t> chunk_t::get_collisions(glm::vec3 _pos, entity_t* _ignored_entity, std::string _tag)
{
	for (auto& entity : entities)
	{
		if (entity->name != _tag)
			continue;

		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags.has(entity_flags_no_collision))
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return entity;
		}
	}


	return std::weak_ptr<entity_t>();
}
