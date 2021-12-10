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
float tick_rate = 1.0f / 12.0f;

void entity_manager::update(double dt) {
	for (auto& _entity : entities) {
		_entity->update_input(dt);
	}

	const float new_time = SDL_GetTicks() / 1000.0f;
	float frame_time = new_time - currentTime;

	if (frame_time > 0.25)
		frame_time = 0.25;

	currentTime = new_time;
	accumulator += frame_time;

	while (accumulator >= tick_rate) {
		step_accumulator++;
		for (auto& _entity : entities) {
			if (step_accumulator % _entity->steps_per_update != 0)
				continue;

			_entity->update_logic();
		}

		accumulator -= tick_rate;
	}

	for (auto& _entity : entities) {
		_entity->update_visuals(dt);
	}
}

void entity_manager::clean() {
	game_event_manager.clear();

	entities.clear();
	entities_tag_lookup.clear();

	name.clear();

	printf("CLEANED LEVEL DATA\n");
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
				ofs << _entity->tag << " " << _entity->grid_pos.x << " " << _entity->grid_pos.y << std::endl;
				/*
				if (_entity->tag == "enemy") {
					ofs << "ENEMY" << " " << (int)_entity->grid_pos.x << " " << (int)_entity->grid_pos.y << std::endl;
				}
				*/
			}
		}
	}

	ofs.close();
}

void entity_manager::load(std::string level_name) {
	printf("RETRIEVING SCENE FILE\n");

	clean();

	std::string levelPath = "data/scenes/";
	levelPath.append(level_name);
	levelPath.append(".scene");

	std::ifstream ifs(levelPath);
	if (ifs.is_open()) {
		printf("PARSING SCENE FILE\n");

		std::string line;
		while (std::getline(ifs, line)) {
			std::stringstream ss(line);
			std::string type;

			std::getline(ss, type, ' ');

			if (type == "FILE_VERSION") {
				uint32_t file_version;
				ss >> file_version;

				if (file_version != fileVersion) {
					printf("OUTDATED LEVEL FILE\n");
					return;
				}
			}

			if (type == "PLAYER") {
				glm::ivec2 position;
				ss >> position.x >> position.y;

				auto p = std::make_shared<player_entity>();
				p->manager = this;
				p->grid_pos = position;

				entities.push_back(p);
				entities_tag_lookup.emplace(p->tag, p);
			}
			else if (type == "ENEMY") {
				glm::ivec2 position;
				ss >> position.x >> position.y;

				auto e = std::make_shared<enemy_entity>();
				e->manager = this;
				e->grid_pos = position;

				entities.push_back(e);
				entities_tag_lookup.emplace(e->tag, e);
			}
		}

		name = level_name;

		printf("FINISHED LOADING SCENE DATA\n");
	}
	else {
		printf("CANNOT FIND SCENE FILE\n");
	}

	ifs.close();
}

bool entity_manager::is_walkable(glm::ivec2 _pos) const {
	if (check_collisions(_pos))
		return false;

	return true;
}

std::vector<glm::ivec2> entity_manager::neighbors(glm::ivec2 _pos) const {
	std::vector<glm::ivec2> results;

	std::vector<glm::ivec2> DIRS = { {0,1}, {0, -1}, {1,0}, {-1, 0} };
	for (auto dir : DIRS) {
		glm::ivec2 next = _pos + dir;
		
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

bool entity_manager::check_collisions(glm::vec2 _pos) const {
	for(auto& entity : entities) {
		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager::check_collisions(glm::vec2 _pos, entity* _ignored_entity) const {
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

bool entity_manager::check_collisions(glm::vec2 _pos, std::string _tag) const {
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

bool entity_manager::check_collisions(glm::vec2 _pos, entity* _ignored_entity, std::string _tag) const {
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

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec2 _pos) {
	for (auto& entity : entities) {
		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		if (entity->grid_pos == common::vec_to_ivec(_pos)) {
			return entity;
		}
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec2 _pos, entity* _ignored_entity) {
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

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec2 _pos, std::string _tag) {
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

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec2 _pos, entity* _ignored_entity, std::string _tag) {
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

//
// Circle Collisions
//

std::vector<std::weak_ptr<entity>> entity_manager::get_circle_collision(glm::vec2 _pos, float _radius) {
	std::vector<std::weak_ptr<entity>> results;

	glm::ivec2 center = common::vec_to_ivec(_pos);
	int top = center.y - _radius,
		bottom = center.y + _radius;

	for (auto& entity : entities) {
		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		for (int y = top; y <= bottom; y++) {
			int dy = y - center.y;
			float dx = sqrt(_radius * _radius - dy * dy);
			int left = ceil(center.x - dx),
				right = floor(center.x + dx);

			for (int x = left; x <= right; x++) {
				glm::ivec2 d = center - entity->grid_pos;
				int distance = d.x * d.x + d.y * d.y;
				if (distance <= _radius * _radius) {
					results.push_back(entity);
				}
			}
		}
	}

	return results;
}

std::vector<std::weak_ptr<entity>> entity_manager::get_circle_collision(glm::vec2 _pos, float _radius, entity* _ignored_entity) {
	std::vector<std::weak_ptr<entity>> results;

	glm::ivec2 center = common::vec_to_ivec(_pos);
	int top = center.y - _radius,
		bottom = center.y + _radius;

	for (auto& entity : entities) {
		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		for (int y = top; y <= bottom; y++) {
			int dy = y - center.y;
			float dx = sqrt(_radius * _radius - dy * dy);
			int left = ceil(center.x - dx),
				right = floor(center.x + dx);

			for (int x = left; x <= right; x++) {
				glm::ivec2 d = center - entity->grid_pos;
				int distance = d.x * d.x + d.y * d.y;
				if (distance <= _radius * _radius) {
					results.push_back(entity);
				}
			}
		}
	}

	return results;
}

std::vector<std::weak_ptr<entity>> entity_manager::get_circle_collision(glm::vec2 _pos, float _radius, std::string _tag) {
	std::vector<std::weak_ptr<entity>> results;

	glm::ivec2 center = common::vec_to_ivec(_pos);
	int top = center.y - _radius,
		bottom = center.y + _radius;

	for (auto& entity : entities) {
		if (entity->tag != _tag)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		for (int y = top; y <= bottom; y++) {
			int dy = y - center.y;
			float dx = sqrt(_radius * _radius - dy * dy);
			int left = ceil(center.x - dx),
				right = floor(center.x + dx);

			for (int x = left; x <= right; x++) {
				glm::ivec2 d = center - entity->grid_pos;
				int distance = d.x * d.x + d.y * d.y;
				if (distance <= _radius * _radius) {
					results.push_back(entity);
				}
			}
		}
	}

	return results;
}

std::vector<std::weak_ptr<entity>> entity_manager::get_circle_collision(glm::vec2 _pos, float _radius, entity* _ignored_entity, std::string _tag) {
	std::vector<std::weak_ptr<entity>> results;

	glm::ivec2 center = common::vec_to_ivec(_pos);
	int top = center.y - _radius,
		bottom = center.y + _radius;

	for (auto& entity : entities) {
		if (entity->tag != _tag)
			continue;

		if (entity.get() == _ignored_entity)
			continue;

		if (entity->flags & ENTITY_NO_COLLISION)
			continue;

		for (int y = top; y <= bottom; y++) {
			int dy = y - center.y;
			float dx = sqrt(_radius * _radius - dy * dy);
			int left = ceil(center.x - dx),
				right = floor(center.x + dx);

			for (int x = left; x <= right; x++) {
				glm::ivec2 d = center - entity->grid_pos;
				int distance = d.x * d.x + d.y * d.y;
				if (distance <= _radius * _radius) {
					results.push_back(entity);
				}
			}
		}
	}

	return results;
}