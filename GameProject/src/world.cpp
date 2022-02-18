#include "world.h"

#include "world_entities.h"

static int current_entity_index = 0;

void world_t::init()
{
	player = *new_entity<player_entity>();
	player.chunk = chunks[current_chunk];
	player.init();

	for(auto& _chunk : chunks)
	{
		for (auto& _entity : _chunk->entities)
		{
			_entity->init();
		}
	}

	log_info("INITIALIZED WORLD");
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

	player.update(dt);
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
				ofs << "\t\tvisual_pos " << _entity->visual_transform.position.x << " " << _entity->visual_transform.position.y << " " << _entity->visual_transform.position.z << std::endl;
				ofs << "\t\tvisual_rot " << _entity->visual_transform.rotation.x << " " << _entity->visual_transform.rotation.y << " " << _entity->visual_transform.rotation.z << std::endl;
				ofs << "\t\tvisual_scl " << _entity->visual_transform.scale.x << " " << _entity->visual_transform.scale.y << " " << _entity->visual_transform.scale.z << std::endl;
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
	//log_info("RETRIEVING WORLD DATA FROM FILE");

	clear_world_data();

	std::string levelPath = "data/world.txt";

	std::ifstream ifs(levelPath);
	if (ifs.is_open())
	{
		//log_info("PARSING WORLD DATA");

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

						if(entity_data.visual_transform.position == glm::vec3(0))
						{
							entity_data.visual_transform.position = entity_data.grid_pos;
						}
					}
					else if(type == "visual_pos")
					{
						ss >> entity_data.visual_transform.position.x;
						ss >> entity_data.visual_transform.position.y;
						ss >> entity_data.visual_transform.position.z;
					}
					else if(type == "visual_rot")
					{
						ss >> entity_data.visual_transform.rotation.x;
						ss >> entity_data.visual_transform.rotation.y;
						ss >> entity_data.visual_transform.rotation.z;
					}
					else if(type == "visual_scl")
					{
						ss >> entity_data.visual_transform.scale.x;
						ss >> entity_data.visual_transform.scale.y;
						ss >> entity_data.visual_transform.scale.z;
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
// HELPER FUNCTIONS
//

template<typename T>
std::shared_ptr<T> new_entity()
{
	std::shared_ptr<entity_t> entity = std::make_shared<T>();

	entity->id = uuid();
	entity->flags = entity_flags_t();
	entity->index = ++current_entity_index;

	return std::dynamic_pointer_cast<T> (entity);
}

void add_entity(std::shared_ptr<chunk_t> chunk, entity_data_t data)
{
	std::shared_ptr<entity_t> entity;

	if (data.type == "player_spawn")
	{
		entity = new_entity<player_spawn_entity>();
	}
	else if (data.type == "block")
	{
		entity = new_entity<block_entity>();
	}
	else
	{
		log_warning("ENTITY TYPE UNKOWN : ", data.type);
		return;
	}

	if (data.id != 0)
	{
		entity->id = data.id;
	}

	entity->chunk = chunk;
	entity->flags = data.flags;
	entity->grid_pos = data.grid_pos;
	entity->visual_transform.position = data.visual_transform.position;
	entity->visual_transform.rotation = data.visual_transform.rotation;
	entity->visual_transform.scale = data.visual_transform.scale;
	chunk->entities.push_back(entity);
}

void add_entity_and_init(std::shared_ptr<chunk_t> chunk, entity_data_t data)
{
	std::shared_ptr<entity_t> entity;

	if (data.type == "player_spawn")
	{
		entity = new_entity<player_spawn_entity>();
	}
	else if (data.type == "block")
	{
		entity = new_entity<block_entity>();
	}
	else
	{
		log_warning("ENTITY TYPE UNKOWN : ", data.type);
		return;
	}

	entity->chunk = chunk;
	entity->flags = data.flags;
	entity->grid_pos = data.grid_pos;
	entity->visual_transform.position = data.visual_transform.position;
	entity->visual_transform.rotation = data.visual_transform.rotation;
	entity->visual_transform.scale = data.visual_transform.scale;
	chunk->entities.push_back(entity);

	entity->init();
}

void remove_entity(std::shared_ptr<chunk_t> chunk, std::weak_ptr<entity_t> _entity)
{
	chunk->entities.erase(std::remove(chunk->entities.begin(), chunk->entities.end(), _entity.lock()), chunk->entities.end());
}

void transition_chunk(std::shared_ptr<chunk_t> next_chunk, entity_t& entity)
{
	entity.chunk = next_chunk;
}
