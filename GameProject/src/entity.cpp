#include "entity.h"

#include <fstream>
#include <sstream>

#include "player.h"
#include "pathfinding.h"

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
	entities_position_lookup.clear();

	for (auto& _entity : entities) {
		entities_position_lookup.emplace(_entity->pos, _entity);
	}

	for (auto& _entity : entities) {
		_entity->update(dt);
	}
}

void entity_manager::clean() {
	game_event_manager.clear();

	entities.clear();
	entities_tag_lookup.clear();
	entities_position_lookup.clear();

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
				entities_tag_lookup.emplace(p->tag, p);
			}
			else if (type == "ENEMY") {
				glm::ivec2 position;
				ss >> position.x >> position.y;

				auto e = std::make_shared<enemy_entity>();
				e->manager = this;
				e->pos = position;

				entities.push_back(e);
				entities_tag_lookup.emplace(e->tag, e);
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

bool entity_manager::is_walkable(glm::ivec2 _pos) const {
	for (auto& entity : entities) {
		if (entity->pos == _pos)
			return false;
	}

	return true;
}

std::vector<glm::ivec2> entity_manager::neighbors(glm::ivec2 _pos) const {
	std::vector<glm::ivec2> results;

	std::vector<glm::ivec2> DIRS = { {0,1}, {0, -1}, {1,0}, {-1, 0} };
	for (auto dir : DIRS) {
		glm::ivec2 next{ _pos.x + dir.x, _pos.y + dir.y };
		
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

bool entity_manager::check_collisions(glm::vec2 _pos) {
	auto range = entities_position_lookup.equal_range(_pos);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second->flags & ENTITY_NO_COLLISION)
			continue;

		if (i->second->pos == common::vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager::check_collisions(glm::vec2 _pos, entity* _ignored_entity) {
	auto range = entities_position_lookup.equal_range(_pos);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second.get() == _ignored_entity)
			continue;

		if (i->second->flags & ENTITY_NO_COLLISION)
			continue;

		if (i->second->pos == common::vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager::check_collisions(glm::vec2 _pos, std::string _tag) {
	// TODO: remove !tag.empty() it probaly isnt needed because
	// it just wont find any entites with empty _tag.
	auto range = entities_position_lookup.equal_range(_pos);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second->tag != _tag)
			continue;

		if (i->second->flags & ENTITY_NO_COLLISION)
			continue;

		if (i->second->pos == common::vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

bool entity_manager::check_collisions(glm::vec2 _pos, entity* _ignored_entity, std::string _tag) {
	auto range = entities_position_lookup.equal_range(_pos);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second->tag != _tag)
			continue;

		if (i->second.get() == _ignored_entity)
			continue;

		if (i->second->flags & ENTITY_NO_COLLISION)
			continue;

		if (i->second->pos == common::vec_to_ivec(_pos)) {
			return true;
		}
	}

	return false;
}

//
// GET COLLISIONS
//

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec2 _pos) {
	auto range = entities_position_lookup.equal_range(_pos);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second->flags & ENTITY_NO_COLLISION)
			continue;

		if (i->second->pos == common::vec_to_ivec(_pos)) {
			return i->second;
		}
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec2 _pos, entity* _ignored_entity) {
	auto range = entities_position_lookup.equal_range(_pos);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second.get() == _ignored_entity)
			continue;

		if (i->second->flags & ENTITY_NO_COLLISION)
			continue;

		if (i->second->pos == common::vec_to_ivec(_pos)) {
			return i->second;
		}
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec2 _pos, std::string _tag) {
	auto range = entities_position_lookup.equal_range(_pos);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second->tag != _tag)
			continue;

		if (i->second->flags & ENTITY_NO_COLLISION)
			continue;

		if (i->second->pos == common::vec_to_ivec(_pos)) {
			return i->second;
		}
	}

	return std::weak_ptr<entity>();
}

std::weak_ptr<entity> entity_manager::get_collisions(glm::vec2 _pos, entity* _ignored_entity, std::string _tag) {
	auto range = entities_position_lookup.equal_range(_pos);
	for (auto i = range.first; i != range.second; ++i) {
		if (i->second->tag != _tag)
			continue;

		if (i->second.get() == _ignored_entity)
			continue;

		if (i->second->flags & ENTITY_NO_COLLISION)
			continue;

		if (i->second->pos == common::vec_to_ivec(_pos)) {
			return i->second;
		}
	}


	return std::weak_ptr<entity>();
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
	if (is_dead) {
		spr->colour = { 1, .5f, 0 };
		ENTITY_FLAG_SET(flags, ENTITY_NO_COLLISION);
		return;
	}

	if (auto player_ref = manager->find_entity_by_tag("player").lock()) {
		std::unordered_map<glm::ivec2, glm::ivec2> came_from;
		std::unordered_map<glm::ivec2, int> cost_so_far;

		a_star_search(*manager, pos, player_ref->pos, came_from, cost_so_far);
		auto path = reconstruct_path(pos, player_ref->pos, came_from);
		for (auto i : path) {
			renderer::debug::draw_circle((glm::vec2)i * (float)renderer::cell_size + (float)renderer::cell_size / 2, 5, colour::green);
		}
	}
}