#include "entity.h"

#include <fstream>
#include <sstream>

#include "world_entities.h"
#include "player.h"

//
// ENTITY MANAGER
//

entity_manager_t::entity_manager_t()
{
}

entity_manager_t::~entity_manager_t()
{
	entities.clear();
	entities_tag_lookup.clear();

	name.clear();

	log_info("CLEANED LEVEL DATA");
}

void entity_manager_t::clear_data()
{
	entities.clear();
	entities_tag_lookup.clear();

	name.clear();
}

float currentTime = SDL_GetTicks() / 1000.0f;
float accumulator = 0.0f;
float tick_rate = 1.0f / 24.0f;

void entity_manager_t::update(double dt)
{
	if (!is_paused)
	{
		for (auto& _entity : entities)
		{
			_entity->update(dt);
		}
	}
}

uint32_t fileVersion = 9;
void entity_manager_t::save() {
	std::string levelPath = "data/scenes/";
	levelPath.append(name);
	levelPath.append(".scene");

	std::ofstream ofs(levelPath, std::ofstream::trunc);
	if (ofs.is_open()) {
		ofs << "file_version " << fileVersion << std::endl;

		if (!entities.empty()) {
			for (auto& _entity : entities) {
				ofs << _entity->name << " " << _entity->id << " " << _entity->flags << " " << _entity->grid_pos.x << " " << _entity->grid_pos.y << " " << _entity->grid_pos.z << std::endl;
			}
		}
	}

	log_info("SAVED SCENE TO FILE");
	ofs.close();
}

void entity_manager_t::load(std::string level_name) {
	log_info("RETRIEVING SCENE FILE");

	clear_data();

	std::string levelPath = "data/scenes/";
	levelPath.append(level_name);
	levelPath.append(".scene");

	std::ifstream ifs(levelPath);
	if (ifs.is_open()) {
		log_info("PARSING SCENE FILE");

		std::string line;
		while (std::getline(ifs, line)) {
			std::stringstream ss(line);
			uint64_t id;
			std::string type;
			ENTITY_FLAGS flags;
			glm::ivec3 grid_pos;

			ss >> type >> id >> flags >> grid_pos.x >> grid_pos.y >> grid_pos.z;

			if (type == "file_version") {
				if (id != fileVersion) {
					log_warning("OUTDATED LEVEL FILE");
					// TODO: maybe remove the return and just let it attempt to parse it.
					return;
				}
			}
			else {
				add_entity(type, id, flags, grid_pos);
			}
		}

		name = level_name;

		log_info("FINISHED LOADING SCENE DATA");
	}
	else {
		log_error("CANNOT FIND SCENE FILE");
	}

	ifs.close();
}

void entity_manager_t::add_entity(std::string _type, uuid _id, ENTITY_FLAGS _flags, glm::ivec3 _grid_pos) {
	std::shared_ptr<entity> _new_entity;

	if (_type == "player") {
		_new_entity = std::make_shared<player_entity>();
	}
	else if (_type == "block") {
		_new_entity = std::make_shared<block_entity>();
	}
	else {
		log_warning("ENTITY TYPE UNKOWN : ", _type);
		return;
	}

	if (_id == 0)
		_id = uuid();

	_new_entity->manager = this;

	_new_entity->id = _id;
	_new_entity->index = ++current_entity_index;
	_new_entity->flags = _flags;
	_new_entity->grid_pos = _grid_pos;
	_new_entity->visual_pos = _new_entity->grid_pos;

	entities.push_back(_new_entity);
	entity_id_lookup.emplace(_new_entity->id, _new_entity);
	entity_index_lookup.emplace(_new_entity->index, _new_entity);
	entities_tag_lookup.emplace(_new_entity->name, _new_entity);
}

void entity_manager_t::remove_entity(std::weak_ptr<entity> _entity) {
	entities.erase(std::remove(entities.begin(), entities.end(), _entity.lock()), entities.end());

	for (auto iter = entities_tag_lookup.begin(); iter != entities_tag_lookup.end();) {
		auto erase_iter = iter++;
		if (erase_iter->second.lock() == _entity.lock())
			entities_tag_lookup.erase(erase_iter);
	}
}

std::vector<glm::ivec3> entity_manager_t::neighbors(glm::ivec3 _pos) const {
	std::vector<glm::ivec3> results;

	std::vector<glm::ivec2> DIRS = { {0,1}, {0, -1}, {1,0}, {-1, 0} };
	for (auto dir : DIRS) {
		glm::ivec3 next = _pos + glm::ivec3(dir,0);
		
		results.push_back(next);
	}

	if ((_pos.x + _pos.y) % 2 == 0)
		std::reverse(results.begin(), results.end());

	return results;
}

std::vector<glm::ivec3> entity_manager_t::diagonal_neighbors(glm::ivec3 _pos) const {
	std::vector<glm::ivec3> results;

	// TODO: change directions to 3D
	std::vector<glm::ivec2> DIRS = { {0,1}, {0, -1}, {1,0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
	for (auto dir : DIRS) {
		glm::ivec3 next = _pos + glm::ivec3(dir, 0);

		results.push_back(next);
	}

	if ((_pos.x + _pos.y) % 2 == 0)
		std::reverse(results.begin(), results.end());

	return results;
}

std::weak_ptr<entity> entity_manager_t::find_entity_by_tag(std::string _tag) const {
	auto range = entities_tag_lookup.equal_range(_tag);

	for (auto i = range.first; i != range.second; ++i) {
		if (i->second.lock()->name == _tag)
			return i->second;
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager_t::find_entity_by_id(uuid _id) const
{
	auto it = entity_id_lookup.find(_id);
	if (it != entity_id_lookup.end())
		return it->second;

	//log_warning("ENTITY WITH ID DOES NOT EXIST");
	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager_t::find_entity_by_index(int _index) const
{
	auto it = entity_index_lookup.find(_index);
	if (it != entity_index_lookup.end())
		return it->second;

	//log_warning("Entity with index does not exist");
	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager_t::find_entity_by_position(glm::vec3 _pos) const
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

bool entity_manager_t::is_entity_at_position(glm::vec3 _pos) const {
	for (auto& entity : entities) {
		if (entity->grid_pos == vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager_t::check_raycast_collision(glm::vec3 _pos, glm::vec3 _dir, float _distance, std::weak_ptr<entity>& _entity) {
	std::map<float, std::weak_ptr<entity>> results;

	glm::vec3 direction = _dir * _distance;

	for (auto& entity : entities)
	{
		float tmin = (entity->grid_pos.x - _pos.x) / direction.x;
		float tmax = ((entity->grid_pos.x + 1) - _pos.x) / direction.x;

		if (tmin > tmax) std::swap(tmin, tmax);

		float tymin = (entity->grid_pos.y - _pos.y) / direction.y;
		float tymax = ((entity->grid_pos.y + 1) - _pos.y) / direction.y;

		if (tymin > tymax) std::swap(tymin, tymax);

		if ((tmin > tymax) || (tymin > tmax))
			continue;

		if (tymin > tmin)
			tmin = tymin;

		if (tymax < tmax)
			tmax = tymax;

		float tzmin = (entity->grid_pos.z - _pos.z) / direction.z;
		float tzmax = ((entity->grid_pos.z + 1) - _pos.z) / direction.z;

		if (tzmin > tzmax) std::swap(tzmin, tzmax);

		if ((tmin > tzmax) || (tzmin > tmax))
			continue;

		if (tzmin > tmin)
			tmin = tzmin;

		if (tzmax < tmax)
			tmax = tzmax;

		float distance = glm::distance(_pos, (glm::vec3)entity->grid_pos);
		results.emplace(distance, entity);
	}

	if (results.empty())
	{
		return false;
	}
	else
	{
		_entity = results.begin()->second;
		return true;
	}
}

bool entity_manager_t::check_collisions(glm::vec3 _pos) const {
	for(auto& entity : entities) {
		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager_t::check_collisions(glm::vec3 _pos, entity* _ignored_entity) const {
	for (auto& entity : entities) {
		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager_t::check_collisions(glm::vec3 _pos, std::string _tag) const {
	for (auto& entity : entities) {
		if (entity->name != _tag)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager_t::check_collisions(glm::vec3 _pos, entity* _ignored_entity, std::string _tag) const {
	for (auto& entity : entities) {
		if (entity->name != _tag)
			continue;

		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

//
// GET COLLISIONS
//

std::weak_ptr<entity> entity_manager_t::get_collisions(glm::vec3 _pos) {
	for (auto& entity : entities) {
		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos)) {
			return entity;
		}
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager_t::get_collisions(glm::vec3 _pos, entity* _ignored_entity) {
	for (auto& entity : entities) {
		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos)) {
			return entity;
		}
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager_t::get_collisions(glm::vec3 _pos, std::string _tag) {
	for (auto& entity : entities) {
		if (entity->name != _tag)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos)) {
			return entity;
		}
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager_t::get_collisions(glm::vec3 _pos, entity* _ignored_entity, std::string _tag) {
	for (auto& entity : entities) {
		if (entity->name != _tag)
			continue;

		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == vec_to_ivec(_pos)) {
			return entity;
		}
	}


	return std::weak_ptr<entity>();
}
