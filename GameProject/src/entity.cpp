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
	for (auto& _entity : entities) {
		if (_entity->stagger_end_time > SDL_GetTicks())
			continue;

		_entity->update(dt);
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

uint32_t fileVersion = 8;
void entity_manager::save() {
	std::string levelPath = "data/scenes/";
	levelPath.append(name);
	levelPath.append(".scene");

	std::ofstream ofs(levelPath, std::ofstream::trunc);
	if (ofs.is_open()) {
		ofs << "FILE_VERSION " << fileVersion << std::endl << std::endl;

		if (!entities.empty()) {
			for (auto& _entity : entities) {
				ofs << _entity->tag << " " << _entity->grid_pos.x << " " << _entity->grid_pos.y << " " << _entity->grid_pos.z << std::endl;
			}
		}
	}

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
			std::string type;

			std::getline(ss, type, ' ');

			if (type == "FILE_VERSION") {
				uint32_t file_version;
				ss >> file_version;

				if (file_version != fileVersion) {
					logger::warning("OUTDATED LEVEL FILE");
					// TODO: maybe remove the return and just let it attempt to parse it.
					return;
				}
			}

			if (type == "player") {
				glm::ivec3 position;
				ss >> position.x >> position.y >> position.z;

				auto p = std::make_shared<player_entity>();
				p->manager = this;
				p->grid_pos = position;

				entities.push_back(p);
				entities_tag_lookup.emplace(p->tag, p);
			}
			else if (type == "block") {
				glm::ivec3 position;
				ss >> position.x >> position.y >> position.z;

				auto b = std::make_shared<block_entity>();
				b->manager = this;
				b->grid_pos = position;

				entities.push_back(b);
				entities_tag_lookup.emplace(b->tag, b);
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

bool entity_manager::is_walkable(glm::ivec3 _pos) const {
	if (check_collisions(_pos))
		return false;

	return true;
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
		if (i->second->tag == _tag)
			return i->second;
	}

	return std::weak_ptr<entity>();
}

//
// CHECK COLLISIONS
//

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