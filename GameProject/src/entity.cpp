#include "entity.h"

#include <fstream>
#include <sstream>

#include "world_entities.h"
#include "player.h"

//
// EVENT MANAGER
//

void event_manager::register_listener(std::string _event_name, listener* _listener) {
	events.emplace(_event_name, _listener);
}

void event_manager::remove_listener(std::string _event_name, listener* _listener) {
	auto range = events.equal_range(_event_name);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second == _listener) {
			events.erase(i);
			break;
		}
	}
}

void event_manager::notify(std::string _event_name) {
	auto range = events.equal_range(_event_name);
	for (auto i = range.first; i != range.second; ++i) {
		i->second->on_notify();
	}
}

void event_manager::clear() {
	events.clear();
}

//
// ENTITY MANAGER
//

void entity_manager::init() {

}

float currentTime = SDL_GetTicks() / 1000.0f;
float accumulator = 0.0f;
float tick_rate = 1.0f / 24.0f;

void entity_manager::update(double dt) {
	if (!is_paused) {
		for (auto& _entity : entities) {
			_entity->update(dt);
		}
	}

	cameras.update(dt);
}

void entity_manager::clean() {
	game_event_manager.clear();

	entities.clear();
	entities_tag_lookup.clear();

	name.clear();

	logger::info("CLEANED LEVEL DATA");
}

uint32_t fileVersion = 9;
void entity_manager::save() {
	std::string levelPath = "data/scenes/";
	levelPath.append(name);
	levelPath.append(".scene");

	std::ofstream ofs(levelPath, std::ofstream::trunc);
	if (ofs.is_open()) {
		ofs << "file_version " << fileVersion << std::endl;

		if (!entities.empty()) {
			for (auto& _entity : entities) {
				ofs << _entity->tag << " " << _entity->id << " " << _entity->flags << " " << _entity->grid_pos.x << " " << _entity->grid_pos.y << " " << _entity->grid_pos.z << std::endl;
			}
		}
	}

	logger::info("SAVED SCENE TO FILE");
	ofs.close();
}

void entity_manager::load(std::string level_name) {
	logger::info("RETRIEVING SCENE FILE");

	clean();

	std::string levelPath = "data/scenes/";
	levelPath.append(level_name);
	levelPath.append(".scene");

	std::ifstream ifs(levelPath);
	if (ifs.is_open()) {
		logger::info("PARSING SCENE FILE");

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
					logger::warning("OUTDATED LEVEL FILE");
					// TODO: maybe remove the return and just let it attempt to parse it.
					return;
				}
			}
			else {
				add_entity(type, id, flags, grid_pos);
			}
		}

		name = level_name;

		logger::info("FINISHED LOADING SCENE DATA");
	}
	else {
		logger::error("CANNOT FIND SCENE FILE");
	}

	ifs.close();
}

void entity_manager::add_entity(std::string _type, uuid _id, ENTITY_FLAGS _flags, glm::ivec3 _grid_pos) {
	std::shared_ptr<entity> _new_entity;

	if (_type == "player") {
		_new_entity = std::make_shared<player_entity>();
	}
	else if (_type == "block") {
		_new_entity = std::make_shared<block_entity>();
	}
	else {
		logger::warning("ENTITY TYPE UNKOWN : ", _type);
		return;
	}
	
	if (_id == 0)
		_id = uuid();

	_new_entity->manager = this;

	_new_entity->id = _id;
	_new_entity->flags = _flags;
	_new_entity->grid_pos = _grid_pos;
	_new_entity->visual_pos = _new_entity->grid_pos;

	entities.push_back(_new_entity);
	entity_id_lookup.emplace(_new_entity->id, _new_entity);
	entities_tag_lookup.emplace(_new_entity->tag, _new_entity);
}

void entity_manager::remove_entity(std::weak_ptr<entity> _entity) {
	entities.erase(std::remove(entities.begin(), entities.end(), _entity.lock()), entities.end());

	for (auto iter = entities_tag_lookup.begin(); iter != entities_tag_lookup.end();) {
		auto erase_iter = iter++;
		if (erase_iter->second.lock() == _entity.lock())
			entities_tag_lookup.erase(erase_iter);
	}
}

std::vector<glm::ivec3> entity_manager::neighbors(glm::ivec3 _pos) const {
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

std::vector<glm::ivec3> entity_manager::diagonal_neighbors(glm::ivec3 _pos) const {
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

std::weak_ptr<entity> entity_manager::find_entity_by_tag(std::string _tag) const {
	auto range = entities_tag_lookup.equal_range(_tag);

	for (auto i = range.first; i != range.second; ++i) {
		if (i->second.lock()->tag == _tag)
			return i->second;
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager::find_entity_by_id(uuid _id) const {
	auto it = entity_id_lookup.find(_id);
	if (it != entity_id_lookup.end())
		return it->second;

	logger::warning("ENTITY WITH ID DOES NOT EXIST");
}

std::weak_ptr<entity> entity_manager::find_entity_by_position(glm::vec3 _pos) const {
	for (auto& entity : entities) {
		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return entity;
		}
	}

	logger::warning("ENTITY AT POSITION DOES NOT EXIST");
}

bool entity_manager::is_entity_at_position(glm::vec3 _pos) const {
	for (auto& entity : entities) {
		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager::check_collisions(glm::vec3 _pos) const {
	for(auto& entity : entities) {
		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager::check_collisions(glm::vec3 _pos, entity* _ignored_entity) const {
	for (auto& entity : entities) {
		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager::check_collisions(glm::vec3 _pos, std::string _tag) const {
	for (auto& entity : entities) {
		if (entity->tag != _tag)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager::check_collisions(glm::vec3 _pos, entity* _ignored_entity, std::string _tag) const {
	for (auto& entity : entities) {
		if (entity->tag != _tag)
			continue;

		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

//
// GET COLLISIONS
//

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec3 _pos) {
	for (auto& entity : entities) {
		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return entity;
		}
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec3 _pos, entity* _ignored_entity) {
	for (auto& entity : entities) {
		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return entity;
		}
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec3 _pos, std::string _tag) {
	for (auto& entity : entities) {
		if (entity->tag != _tag)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return entity;
		}
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec3 _pos, entity* _ignored_entity, std::string _tag) {
	for (auto& entity : entities) {
		if (entity->tag != _tag)
			continue;

		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return entity;
		}
	}


	return std::weak_ptr<entity>();
}