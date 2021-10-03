#include "scene.h"

#include <fstream>
#include <sstream>

#include "player.h"

level::level_data level::data;

void level::init() {

}

void level::update(double dt) {
	for (auto _obstacle : data.obstacles) {
		_obstacle->update(dt);
	}
}

void level::clean() {
	data.obstacles.clear();

	// TODO: maybe delete sprites from existence on level clear. maybe it might not be worth it.

	data.sprites.clear();

	printf("CLEANED LEVEL DATA\n");
}

unsigned int fileVersion = 1;
void level::save(std::string level_name) {
	std::string levelPath = "data/scenes/" + level_name;

	std::ofstream ofs(levelPath, std::ofstream::trunc);
	if (ofs.is_open()) {
		ofs << "FILE_VERSION " << fileVersion << std::endl << std::endl;

		if (!data.obstacles.empty()) {
			for (auto& _obstacle : data.obstacles) {
				ofs << "OBSTACLE " << (int)_obstacle->pos.x << " " << (int)_obstacle->pos.y << std::endl;
			}

			ofs << std::endl;
		}

		if (!data.path_nodes.empty()) {
			for (auto& _node : data.path_nodes) {
				ofs << "PATH_NODE " << (int)_node.x << " " << (int)_node.y << std::endl;
			}

			ofs << std::endl;
		}

		if (!data.sprites.empty()) {
			for (auto& _sprite : data.sprites) {
				ofs << "SPRITE " << (int)_sprite->position->x << " " << (int)_sprite->position->y << std::endl;
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

	std::string levelPath = "data/scenes/" + level_name;

	std::ifstream ifs(levelPath);
	if (ifs.is_open()) {
		printf("PARSING SCENE FILE\n");

		std::string line;
		while (std::getline(ifs, line)) {
			std::stringstream ss(line);
			std::string type;
			
			std::getline(ss, type, ' ');

			if (type == "FILE_VERSION") {
				int file_version;
				ss >> file_version;

				if (file_version != fileVersion) {
					printf("OUTDATED LEVEL FILE\n");
					return;
				}
			}

			if (type == "OBSTACLE") {
				glm::ivec2 position;
				ss >> position.x >> position.y;

				auto o = std::make_shared<obstacle>();
				o->pos = position;

				data.obstacles.push_back(o);
			}
			else if (type == "PATH_NODE") {
				glm::vec2 node;
				ss >> node.x >> node.y;
				data.path_nodes.push_back(node);
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