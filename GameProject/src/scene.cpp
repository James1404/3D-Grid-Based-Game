#include "scene.h"

#include <fstream>
#include <sstream>

#include "player.h"

level current_level;

//
// EVENTS
//

void event_manager::register_listener(std::string _event_name, listener* _listener) {
	events.emplace(_event_name, _listener);
}

void event_manager::remove_listener(std::string _event_name, listener* _listener) {
	auto range = events.equal_range(_event_name);
	for(auto i = range.first; i != range.second; ++i) {
		if(i->second == _listener) {
			events.erase(i);
			break;
		}
	}
}

void event_manager::notify(std::string _event_name) {
	auto range = events.equal_range(_event_name);
	for(auto i = range.first; i != range.second; ++i) {
		i->second->on_notify();
	}
}

void event_manager::clear() {
	events.clear();
}

//
// LISTENERS 
//

cutscene::cutscene() {
	current_level.game_event_manager.register_listener(event_name, this);
}

cutscene::~cutscene() {
	current_level.game_event_manager.remove_listener(event_name, this);
}

void cutscene::update(double dt) {
	if (is_active) {
		// start cutscene
	}
}

void cutscene::on_notify() {
	if (is_active)
		return;

	printf("Started Cutscene %p %s\n", this, event_name.c_str());
	is_active = true;
}

//
// LEVEL
//

void level::init() {

}

void level::update(double dt) {
	for (auto& _obstacle : obstacles) {
		_obstacle->update(dt);
	}

	for (auto& _enemy : enemies) {
		_enemy->update(dt);
	}
	
	for (auto& _sprite : sprites) {
		_sprite->update(dt);
	}

	for (auto& _cutscene : cutscenes) {
		_cutscene->update(dt);
	}
}

void level::clean() {
	game_event_manager.clear();

	obstacles.clear();
	enemies.clear();
	sprites.clear();
	cutscenes.clear();

	name.clear();

	printf("CLEANED LEVEL DATA\n");
}

uint32_t fileVersion = 8;
void level::save() {
	std::string levelPath = "data/scenes/";
	levelPath.append(name);
	levelPath.append(".scene");

	std::ofstream ofs(levelPath, std::ofstream::trunc);
	if (ofs.is_open()) {
		ofs << "FILE_VERSION " << fileVersion << std::endl << std::endl;

		if (!obstacles.empty()) {
			for (auto& _obstacle : obstacles) {
				ofs << "OBSTACLE" << " " << (int)_obstacle->pos.x << " " << (int)_obstacle->pos.y << std::endl;
			}

			ofs << std::endl;
		}

		if (!enemies.empty()) {
			for (auto& _enemy : enemies) {
				ofs << "ENEMY" << " " << (int)_enemy->pos.x << " " << (int)_enemy->pos.y << std::endl;
			}

			ofs << std::endl;
		}

		if (!sprites.empty()) {
			for (auto& _sprite : sprites) {
				ofs << "SPRITE" << " " << (int)_sprite->pos.x << " " << (int)_sprite->pos.y << " " <<
					(int)_sprite->spr->layer << " " <<
					_sprite->spr->colour.x << " " << _sprite->spr->colour.y << " " << _sprite->spr->colour.z << std::endl;
			}

			ofs << std::endl;
		}

		if(!cutscenes.empty()){
			for (auto& _cutscene : cutscenes) {
				ofs << "CUTSCENE" << " " << _cutscene->event_name << " " << std::endl;
			}

			ofs << std::endl;
		}
	}

	ofs.close();
}

void level::load(std::string level_name) {
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

			if (type == "OBSTACLE") {
				glm::ivec2 position;
				ss >> position.x >> position.y;

				auto o = std::make_shared<obstacle_entity>();
				o->pos = position;

				obstacles.push_back(o);
			}
			else if (type == "ENEMY") {
				glm::ivec2 position;
				// int node;
				ss >> position.x >> position.y; // >> node;

				auto e = std::make_shared<enemy_entity>();
				e->pos = position;

				enemies.push_back(e);
			}
			else if (type == "SPRITE") {
				glm::ivec2 position;
				int layer;
				glm::vec3 colour;
				//std::string path;

				ss >> position.x >> position.y >> layer >> colour.x >> colour.y >> colour.z;

				auto s = std::make_shared<sprite_entity>();
				s->pos = position;
				s->spr->layer = layer;
				s->spr->colour = colour;
				// s->sprite_path = path;
				// s->spr->set_sprite_path(s->sprite_path.c_str());

				sprites.push_back(s);
			}
			else if (type == "CUTSCENE") {
				std::string event_name;

				ss >> event_name;

				auto c = std::make_shared<cutscene>();
				c->event_name = event_name;

				cutscenes.push_back(c);
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
