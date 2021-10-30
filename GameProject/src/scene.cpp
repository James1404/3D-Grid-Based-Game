#include "scene.h"

#include <fstream>
#include <sstream>

#include "player.h"

level current_level;

//
// EVENTS
//

void game_event::register_function(callback_function func) {
	registered_functions.push_back(*func);
}

void game_event::remove_function(callback_function func) {
	registered_functions.erase(std::remove(registered_functions.begin(), registered_functions.end(), func), registered_functions.end());
}

void game_event::notify() {
	for(auto& func : registered_functions) {
			func();
	}
}

bool find_game_event(game_event* _event, std::string _name)  {
	for(auto& e : current_level.game_events) {
		if(e->event_name == _name) {
			_event = e.get();
			return true;
		}	
	}
	return false;
}

//
// CUTSCENE
//

void cutscene::init() {

}

void cutscene::clean() {

}

void cutscene::start_cutscene() {

}

void cutscene::update(double dt) {

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

	for (auto& _sprite : sprites) {
		_sprite->update(dt);
	}

	for (auto& _enemy : enemies) {
		_enemy->update(dt);
	}
}

void level::clean() {
	game_events.clear();
	path_nodes.clear();
	cutscenes.clear();

	entities.clear();

	obstacles.clear();
	enemies.clear();

	// TODO: maybe delete sprites from existence on level clear. maybe it might not be worth it.
	sprites.clear();

	name.clear();

	printf("CLEANED LEVEL DATA\n");
}

uint32_t fileVersion = 5;
void level::save() {
	std::string levelPath = "data/scenes/";
	levelPath.append(name);
	levelPath.append(".scene");

	std::ofstream ofs(levelPath, std::ofstream::trunc);
	if (ofs.is_open()) {
		ofs << "FILE_VERSION " << fileVersion << std::endl << std::endl;

		if (!game_events.empty()) {
			for (auto& _event : game_events) {
				ofs << "EVENT" << " " << _event->event_name << std::endl;
			}

			ofs << std::endl;
		}

		if (!obstacles.empty()) {
			for (auto& _obstacle : obstacles) {
				ofs << "OBSTACLE" << " " << (int)_obstacle->pos.x << " " << (int)_obstacle->pos.y << std::endl;
			}

			ofs << std::endl;
		}

		if (!path_nodes.empty()) {
			for (auto& _node : path_nodes) {
				if(_node->is_trigger)
					ofs << "PATH_NODE" << " " << (int)_node->pos.x << " " << (int)_node->pos.y << " " << _node->flags << " " << _node->is_trigger << " " << _node->event_name << std::endl;
				else
					ofs << "PATH_NODE" << " " << (int)_node->pos.x << " " << (int)_node->pos.y << " " << _node->flags << " " << _node->is_trigger << " " << "NULL" << std::endl;
			}

			ofs << std::endl;
		}

		if (!enemies.empty()) {
			for (auto& _enemy : enemies) {
				ofs << "ENEMY" << " " << (int)_enemy->pos.x << " " << (int)_enemy->pos.y << " " << _enemy->current_node << std::endl;
			}

			ofs << std::endl;
		}

		if (!sprites.empty()) {
			for (auto& _sprite : sprites) {
				ofs << "SPRITE" << " " << (int)_sprite->pos.x << " " << (int)_sprite->pos.y << " " <<
					(int)_sprite->spr->size.x << " " << (int)_sprite->spr->size.y << " " <<
					(int)_sprite->spr->layer << " " <<
					_sprite->spr->colour.x << " " << _sprite->spr->colour.y << " " << _sprite->spr->colour.z << std::endl;
			}

			ofs << std::endl;
		}

		if(!cutscenes.empty()){
			for (auto& _cutscene : cutscenes) {
				ofs << "CUTSCENE" << " " << std::endl;
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

			if (type == "EVENT") {
				std::string name;

				ss >> name;

				auto e = std::make_shared<game_event>();
				e->event_name = name;

				game_events.push_back(e);
			}
			else if (type == "OBSTACLE") {
				glm::ivec2 position;
				ss >> position.x >> position.y;

				auto o = std::make_shared<obstacle_entity>();
				o->pos = position;

				obstacles.push_back(o);
			}
			else if (type == "PATH_NODE") {
				glm::vec2 pos;
				PATH_NODE_FLAGS flags;
				ss >> pos.x >> pos.y >> flags;

				auto n = std::make_shared<path_node>();
				n->pos = pos;
				n->flags = flags;

				path_nodes.push_back(n);
			}
			else if (type == "ENEMY") {
				glm::ivec2 position;
				int node;
				ss >> position.x >> position.y >> node;

				auto e = std::make_shared<enemy_entity>();
				e->pos = position;
				e->current_node = node;

				enemies.push_back(e);
			}
			else if (type == "SPRITE") {
				glm::ivec2 position;
				glm::ivec2 size;
				int layer;
				glm::vec3 colour;
				//std::string path;

				ss >> position.x >> position.y
					>> size.x >> size.y >>
					layer >>
					colour.x >> colour.y >> colour.z;

				auto s = std::make_shared<sprite_entity>();
				s->pos = position;
				s->spr->size = size;
				s->spr->layer = layer;
				s->spr->colour = colour;
				// s->sprite_path = path;
				// s->spr->set_sprite_path(s->sprite_path.c_str());

				sprites.push_back(s);
			}
			else if (type == "CUTSCENE") {
				auto c = std::make_shared<cutscene>();

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
