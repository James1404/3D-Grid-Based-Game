#include "entity.h"

#include "player.h"

#include <fstream>
#include <sstream>

//
// EVENTS
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
// EVENT MANAGER
//

void entity_manager::init() {

}

void entity_manager::update(double dt) {
	for (auto& _entity : entities) {
		_entity->update(dt);
	}
}

void entity_manager::clean() {
	game_event_manager.clear();

	entities.clear();
	entities_quick_tag_lookup.clear();

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
				if (_entity->tag == "enemy") {
					ofs << "ENEMY" << " " << (int)_entity->pos.x << " " << (int)_entity->pos.y << std::endl;
				}
			}

			ofs << std::endl;
		}
	}

	ofs.close();
}

void entity_manager::load(std::string level_name) {
	printf("---------------------\n");
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
				p->pos = position;

				entities.push_back(p);
				entities_quick_tag_lookup.emplace(p->tag, p.get());
			}
			else if (type == "ENEMY") {
				glm::ivec2 position;
				ss >> position.x >> position.y;

				auto e = std::make_shared<enemy_entity>();
				e->manager = this;
				e->pos = position;

				entities.push_back(e);
				entities_quick_tag_lookup.emplace(e->tag, e.get());
			}
		}

		name = level_name;

		printf("FINISHED LOADING SCENE DATA\n");
		printf("---------------------------\n");
	}
	else {
		printf("CANNOT FIND SCENE FILE\n");
	}

	ifs.close();
}

bool entity_manager::check_collisions(glm::vec2 _pos, std::string _tag) {
	if (_tag.empty()) {
		for (const auto& entity : entities) {
			if (entity->flags & ENTITY_NO_COLLISION)
				continue;

			if (entity->pos == common::vec_to_ivec(_pos)) {
				return true;
			}
		}
	}
	else {
		auto range = entities_quick_tag_lookup.equal_range(_tag);
		for (auto i = range.first; i != range.second; ++i) {
			if (i->second->flags & ENTITY_NO_COLLISION)
				continue;

			if (i->second->pos == common::vec_to_ivec(_pos)) {
				return true;
			}
		}
	}

	return false;
}

bool entity_manager::check_collisions(entity* _owner, glm::vec2 _pos, std::string _tag) {
	if (_tag.empty()) {
		for (const auto& entity : entities) {
			if (entity.get() == _owner)
				continue;

			if (entity->flags & ENTITY_NO_COLLISION)
				continue;

			if (entity->pos == common::vec_to_ivec(_pos)) {
				return true;
			}
		}
	}
	else {
		auto range = entities_quick_tag_lookup.equal_range(_tag);
		for (auto i = range.first; i != range.second; ++i) {
			if (i->second == _owner)
				continue;

			if (i->second->flags & ENTITY_NO_COLLISION)
				continue;

			if (i->second->pos == common::vec_to_ivec(_pos)) {
				return true;
			}
		}
	}

	return false;
}

entity* entity_manager::get_collisions(glm::vec2 _pos, std::string _tag) {
	if (_tag.empty()) {
		for (const auto& entity : entities) {
			if (entity->flags & ENTITY_NO_COLLISION)
				continue;

			if (entity->pos == common::vec_to_ivec(_pos)) {
				return entity.get();
			}
		}
	}
	else {
		auto range = entities_quick_tag_lookup.equal_range(_tag);
		for (auto i = range.first; i != range.second; ++i) {
			if (i->second->flags & ENTITY_NO_COLLISION)
				continue;

			if (i->second->pos == common::vec_to_ivec(_pos)) {
				return i->second;
			}
		}
	}

	return nullptr;
}

entity* entity_manager::get_collisions(entity* _owner, glm::vec2 _pos, std::string _tag) {
	if (_tag.empty()) {
		for (const auto& entity : entities) {
			if (entity.get() == _owner)
				continue;

			if (entity->flags & ENTITY_NO_COLLISION)
				continue;

			if (entity->pos == common::vec_to_ivec(_pos)) {
				return entity.get();
			}
		}
	}
	else {
		auto range = entities_quick_tag_lookup.equal_range(_tag);
		for (auto i = range.first; i != range.second; ++i) {
			if (i->second == _owner)
				continue;

			if (i->second->flags & ENTITY_NO_COLLISION)
				continue;

			if (i->second->pos == common::vec_to_ivec(_pos)) {
				return i->second;
			}
		}
	}

	return nullptr;
}

//
// ENTITIES
//

enemy_entity::enemy_entity() {
	tag = "enemy";

	spr = renderer::create_sprite();
	spr->position = &pos;
	spr->layer = -1;
	//spr->set_sprite_path("player.png");
	spr->colour = { 1,0,0 };

	is_dead = false;

	current_health_points = max_health_points;
}

enemy_entity::~enemy_entity() {
	renderer::delete_sprite(spr);
}

void enemy_entity::update(double dt) {
	if (is_dead)
		return;
}