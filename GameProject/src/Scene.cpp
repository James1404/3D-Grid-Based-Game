#include "scene.h"

#include <fstream>
#include <sstream>

level runtime_level;

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
}

void level::clean() {
	obstacles.clear();
	enemies.clear();

	// TODO: maybe delete sprites from existence on level clear. maybe it might not be worth it.

	sprites.clear();

	printf("CLEANED LEVEL DATA\n");
}

uint32_t fileVersion = 1;
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

		if (!path_nodes.empty()) {
			for (auto& _node : path_nodes) {
				ofs << "PATH_NODE" << " " << (int)_node.x << " " << (int)_node.y << std::endl;
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
				ofs << "SPRITE" << " " << (int)_sprite->pos.x << " " << (int)_sprite->pos.y << " " << (int)_sprite->spr->layer << " " << _sprite->sprite_path << std::endl;
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
			else if (type == "PATH_NODE") {
				glm::vec2 node;
				ss >> node.x >> node.y;
				path_nodes.push_back(node);
			}
			else if (type == "ENEMY") {
				glm::ivec2 position;
				ss >> position.x >> position.y;

				auto e = std::make_shared<enemy_entity>();
				e->pos = position;

				enemies.push_back(e);
			}
			else if (type == "SPRITE") {
				glm::ivec2 position;
				int layer;
				std::string path;

				ss >> position.x >> position.y >> layer >> path;

				auto s = std::make_shared<sprite_entity>();
				s->pos = position;
				s->spr->layer = layer;
				s->sprite_path = path;
				s->spr->set_sprite_path(s->sprite_path.c_str());

				sprites.push_back(s);
			}
		}

		printf("FINISHED LOADING SCENE DATA\n");
		printf("---------------------------\n");
	}
	else {
		printf("CANNOT FIND SCENE FILE\n");
	}

	ifs.close();
}