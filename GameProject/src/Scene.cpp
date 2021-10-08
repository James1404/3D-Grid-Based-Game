#include "scene.h"

#include <fstream>
#include <sstream>

#include "player.h"

level::level_data level::data;

void level::init() {

}

void level::update(double dt) {
	for (auto& _obstacle : data.obstacles) {
		_obstacle->update(dt);
	}

	for (auto& _enemy : data.enemies) {
		_enemy->update(dt);
	}

	for (auto& _sprite : data.sprites) {
		_sprite->update(dt);
	}
}

void level::clean() {
	data.obstacles.clear();
	data.enemies.clear();
	data.path_nodes.clear();

	// TODO: maybe delete sprites from existence on level clear. maybe it might not be worth it.
	data.sprites.clear();

	printf("CLEANED LEVEL DATA\n");
}

uint32_t fileVersion = 2;
void level::save() {
	std::string levelPath = "data/scenes/";
	levelPath.append(data.name);
	levelPath.append(".scene");

	std::ofstream ofs(levelPath, std::ofstream::trunc);
	if (ofs.is_open()) {
		ofs << "FILE_VERSION " << fileVersion << std::endl << std::endl;

		if (!data.obstacles.empty()) {
			for (auto& _obstacle : data.obstacles) {
				ofs << "OBSTACLE" << " " << (int)_obstacle->pos.x << " " << (int)_obstacle->pos.y << std::endl;
			}

			ofs << std::endl;
		}

		if (!data.path_nodes.empty()) {
			for (auto& _node : data.path_nodes) {
				ofs << "PATH_NODE" << " " << (int)_node.x << " " << (int)_node.y << std::endl;
			}

			ofs << std::endl;
		}

		if (!data.enemies.empty()) {
			for (auto& _enemy : data.enemies) {
				ofs << "ENEMY" << " " << (int)_enemy->pos.x << " " << (int)_enemy->pos.y << std::endl;
			}

			ofs << std::endl;
		}

		if (!data.sprites.empty()) {
			for (auto& _sprite : data.sprites) {
				ofs << "SPRITE" << " " << (int)_sprite->pos.x << " " << (int)_sprite->pos.y << " " <<
					(int)_sprite->spr->size.x << " " << (int)_sprite->spr->size.y << " " <<
					(int)_sprite->spr->layer << " " <<
					_sprite->spr->colour.x << " " << _sprite->spr->colour.y << " " << _sprite->spr->colour.z << std::endl;
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

				data.obstacles.push_back(o);
			}
			else if (type == "PATH_NODE") {
				glm::vec2 node;
				ss >> node.x >> node.y;
				data.path_nodes.push_back(node);
			}
			else if (type == "ENEMY") {
				glm::ivec2 position;
				ss >> position.x >> position.y;

				auto e = std::make_shared<enemy_entity>();
				e->pos = position;

				data.enemies.push_back(e);
			}
			else if (type == "SPRITE") {
				glm::ivec2 position;
				glm::ivec2 size;
				int layer;
				glm::ivec3 colour;
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

				data.sprites.push_back(s);
			}
		}

		player::data.pos = data.path_nodes[0];
		player::data.current_node = 0;

		data.name = level_name;

		printf("FINISHED LOADING SCENE DATA\n");
		printf("---------------------------\n");
	}
	else {
		printf("CANNOT FIND SCENE FILE\n");
	}

	ifs.close();
}