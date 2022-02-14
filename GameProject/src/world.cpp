#include "world.h"

#include "player.h"
#include "world_entities.h"

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

bool entity::is_grounded(glm::ivec3 _pos)
{
	return chunk->check_collisions(_pos + glm::ivec3(0, -1, 0), this);
}

bool entity::is_moving() const
{
	return (visual_pos != (glm::vec3)grid_pos);
}

void entity::move_grid_pos(glm::ivec3 _dir)
{
	if (is_moving())
		return;

	glm::ivec3 new_pos = grid_pos + _dir;
	if (is_grounded(grid_pos))
	{
		if (!chunk->check_collisions(new_pos, this))
		{
			if (is_grounded(new_pos))
			{
				set_grid_pos(new_pos);
			}
			else
			{
				if (!chunk->check_collisions(new_pos + glm::ivec3(0, -1, 0)))
				{
					if (is_grounded(new_pos + glm::ivec3(0, -1, 0)))
						set_grid_pos(new_pos + glm::ivec3(0, -1, 0));
				}
			}
		}
		else
		{
			// TODO: make no climb work
			if (!chunk->check_collisions(grid_pos + glm::ivec3(0, 1, 0)))
			{
				if (!chunk->check_collisions(new_pos + glm::ivec3(0, 1, 0)))
				{
					set_grid_pos(new_pos + glm::ivec3(0, 1, 0));
				}
			}
		}
	}
}

void entity::set_grid_pos(glm::vec3 _pos)
{
	previous_grid_pos = grid_pos;
	grid_pos = _pos;
}

void entity::revert_grid_pos()
{
	grid_pos = previous_grid_pos;
}

void entity::interp_visuals(double dt, float interp_speed)
{
	// TODO: implement lerping instead of move_towards
	//visual_pos = lerp(visual_pos, (glm::vec3)grid_pos, interp_speed * dt);
	visual_pos = move_towards(visual_pos, grid_pos, interp_speed * dt);
}

//
// WORLD
//

static int current_entity_index = 0;

void world_t::init()
{
	for(auto& _chunk : chunks)
	{
		for (auto& _entity : _chunk->entities)
		{
			_entity->init();
		}
	}
}

void world_t::shutdown()
{
	clear_world_data();

	log_info("CLEANED LEVEL DATA");
}

std::shared_ptr<chunk_t> world_t::get_current_chunk()
{
	return chunks[current_chunk];
}

void world_t::update(double dt)
{
	if (is_paused)
		return;

	for(auto& chunk : chunks)
	{
		for(auto& _entity : chunk->entities)
		{
			_entity->update(dt);
		}
	}
	/*
	for (auto& _entity : chunks[current_chunk].entities)
	{
		_entity->update(dt);
	}
	*/
}

void world_t::clear_world_data()
{
	for(auto& _chunk : chunks)
	{
		_chunk->clear_data();
	}

	chunks.clear();
}

int fileVersion = 10;
void world_t::save()
{
	std::string levelPath = "data/world.txt";

	std::ofstream ofs(levelPath, std::ofstream::trunc);
	if (ofs.is_open())
	{
		ofs << "file_version " << fileVersion << std::endl;

		for (auto& chunk : chunks)
		{
			ofs << "chunk " << chunk->name << std::endl;
			for(auto& _entity : chunk->entities)
			{
				ofs << "\tentity " << _entity->name << std::endl;
				ofs << "\t\tid " << _entity->id << std::endl;
				ofs << "\t\tflags " << _entity->flags << std::endl;
				ofs << "\t\tgrid_pos " << _entity->grid_pos.x << " " << _entity->grid_pos.y << " " << _entity->grid_pos.z << std::endl;
				ofs << "\t\tvisual_pos " << _entity->visual_pos.x << " " << _entity->visual_pos.y << " " << _entity->visual_pos.z << std::endl;
				ofs << "\t\tvisual_rot " << _entity->visual_rot.x << " " << _entity->visual_rot.y << " " << _entity->visual_rot.z << std::endl;
				ofs << "\t\tvisual_scl " << _entity->visual_scl.x << " " << _entity->visual_scl.y << " " << _entity->visual_scl.z << std::endl;
				ofs << "\tend_entity" << std::endl;
			}
			ofs << "end_chunk" << std::endl;
		}
	}

	log_info("SAVED WORLD TO FILE");
	ofs.close();
}

void world_t::load()
{
	log_info("RETRIEVING WORLD DATA FROM FILE");

	clear_world_data();

	std::string levelPath = "data/world.txt";

	std::ifstream ifs(levelPath);
	if (ifs.is_open())
	{
		log_info("PARSING WORLD DATA");

		entity_data_t entity_data;

		bool in_chunk = false;
		bool in_entity = false;
		bool in_comment = false;

		std::string line;
		while (std::getline(ifs, line))
		{
			std::stringstream ss(line);
			std::string type;

			ss >> type;

			if(type == "//")
			{
				continue;
			}
			else if (type == "/*")
			{
				in_comment = true;
			}
			else if(type == "*/")
			{
				if(in_comment)
				{
					in_comment = false;
				}
			}

			if(in_comment)
			{
				continue;
			}

			if (type == "file_version")
			{
				int version;	
				ss >> version;
				if (version != fileVersion)
				{
					log_warning("OUTDATED LEVEL FILE");
					return;
				}
			}
			else if(type == "chunk")
			{
				auto chunk = std::make_shared<chunk_t>();
				ss >> chunk->name;
				chunks.push_back(chunk);

				in_chunk = true;
			}
			if(in_chunk)
			{
				if(type == "entity")
				{
					entity_data = entity_data_t();
					ss >> entity_data.type;

					in_entity = true;
				}
				else if(type == "end_chunk")
				{
					in_entity = false;
					in_chunk = false;
				}

				if(in_entity)
				{
					if(type == "id")
					{
						uint64_t id;
						ss >> id;
						entity_data.id = id;
					}
					else if(type == "flags")
					{
						uint32_t flags;
						ss >> flags;
						entity_data.flags = flags;
					}
					else if (type == "grid_pos")
					{
						ss >> entity_data.grid_pos.x;
						ss >> entity_data.grid_pos.y;
						ss >> entity_data.grid_pos.z;

						if(entity_data.visual_pos == glm::vec3(0))
						{
							entity_data.visual_pos = entity_data.grid_pos;
						}
					}
					else if(type == "visual_pos")
					{
						ss >> entity_data.visual_pos.x;
						ss >> entity_data.visual_pos.y;
						ss >> entity_data.visual_pos.z;
					}
					else if(type == "visual_rot")
					{
						ss >> entity_data.visual_rot.x;
						ss >> entity_data.visual_rot.y;
						ss >> entity_data.visual_rot.z;
					}
					else if(type == "visual_scl")
					{
						ss >> entity_data.visual_scl.x;
						ss >> entity_data.visual_scl.y;
						ss >> entity_data.visual_scl.z;
					}
					else if(type == "end_entity")
					{
						add_entity(chunks.back(), entity_data);
						in_entity = false;
					}
				}
			}
		}

		//name = level_name;

		log_info("FINISHED LOADING WORLD DATA");
	}
	else
	{
		log_error("CANNOT FIND WORLD FILE");
	}

	ifs.close();

	init();
}

int world_t::new_chunk()
{
	auto chunk = std::make_shared<chunk_t>();
	chunks.push_back(chunk);
	return chunks.size() - 1;
}

void world_t::delete_chunk(int index)
{
	if(chunks.empty())
		return;

	chunks[index]->clear_data();
	if(index < chunks.size() - 1)
	{
		chunks.erase(chunks.begin() + index);
	}
	else
	{
		if(index == current_chunk)
		{
			current_chunk--;
		}

		chunks.pop_back();

		if(chunks.size() < 1)
		{
			current_chunk = new_chunk();
		}
	}
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

std::weak_ptr<entity> chunk_t::find_entity_by_id(uuid _id) const
{
	for (auto& entity : entities)
	{
		if (entity->id == _id)
		{
			return entity;
		}
	}

	//log_warning("ENTITY WITH ID DOES NOT EXIST");
	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> chunk_t::find_entity_by_index(int _index) const
{
	for (auto& entity : entities)
	{
		if (entity->index == _index)
		{
			return entity;
		}
	}

	//log_warning("Entity with index does not exist");
	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> chunk_t::find_entity_by_position(glm::vec3 _pos) const
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

std::weak_ptr<entity> chunk_t::get_entity_at_position(glm::vec3 _pos)
{
	for (auto& entity : entities)
	{
		if (entity->grid_pos == vec_to_ivec(_pos))
		{
			return entity;
		}
	}

	return std::weak_ptr<entity>();
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

bool chunk_t::check_collisions(glm::vec3 _pos, entity* _ignored_entity) const
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

bool chunk_t::check_collisions(glm::vec3 _pos, entity* _ignored_entity, std::string _tag) const {
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

//
// GET COLLISIONS
//

std::weak_ptr<entity> chunk_t::get_collisions(glm::vec3 _pos)
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

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> chunk_t::get_collisions(glm::vec3 _pos, entity* _ignored_entity)
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

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> chunk_t::get_collisions(glm::vec3 _pos, std::string _tag)
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

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> chunk_t::get_collisions(glm::vec3 _pos, entity* _ignored_entity, std::string _tag)
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


	return std::weak_ptr<entity>();
}

//
// HELPER FUNCTIONS
//

template<typename T>
std::weak_ptr<entity> new_entity(std::shared_ptr<chunk_t> chunk)
{
	std::shared_ptr<entity> entity = std::make_shared<T>();
	
	entity->chunk = chunk;

	entity->id = uuid();
	entity->flags = entity_flags_t();
	entity->index = ++current_entity_index;

	//entity->init();

	chunk->entities.push_back(entity);

	return entity;
}

void add_entity(std::shared_ptr<chunk_t> chunk, entity_data_t data)
{
	std::weak_ptr<entity> entity;

	if (data.type == "player")
	{
		entity = new_entity<player_entity>(chunk);
	}
	else if (data.type == "block")
	{
		entity = new_entity<block_entity>(chunk);
	}
	else
	{
		log_warning("ENTITY TYPE UNKOWN : ", data.type);
		return;
	}

	if (auto tmp_entity = entity.lock())
	{
		if (data.id != 0)
		{
			tmp_entity->id = data.id;
		}

		tmp_entity->flags = data.flags;
		tmp_entity->grid_pos = data.grid_pos;
		tmp_entity->visual_pos = data.visual_pos;
		tmp_entity->visual_rot = data.visual_rot;
		tmp_entity->visual_scl = data.visual_scl;
	}
}

void add_entity_and_init(std::shared_ptr<chunk_t> chunk, entity_data_t data)
{
	std::weak_ptr<entity> entity;

	if (data.type == "player")
	{
		entity = new_entity<player_entity>(chunk);
	}
	else if (data.type == "block")
	{
		entity = new_entity<block_entity>(chunk);
	}
	else
	{
		log_warning("ENTITY TYPE UNKOWN : ", data.type);
		return;
	}

	if (auto tmp_entity = entity.lock())
	{
		if (data.id != 0)
		{
			tmp_entity->id = data.id;
		}

		tmp_entity->flags = data.flags;
		tmp_entity->grid_pos = data.grid_pos;
		tmp_entity->visual_pos = data.visual_pos;
		tmp_entity->visual_rot = data.visual_rot;
		tmp_entity->visual_scl = data.visual_scl;

		tmp_entity->init();
	}
}

void remove_entity(std::shared_ptr<chunk_t> chunk, std::weak_ptr<entity> _entity)
{
	chunk->entities.erase(std::remove(chunk->entities.begin(), chunk->entities.end(), _entity.lock()), chunk->entities.end());
}

void transition_chunk(std::shared_ptr<chunk_t> next_chunk, entity& entity)
{
	entity.chunk = next_chunk;
}
