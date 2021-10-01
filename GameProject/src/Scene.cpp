#include "scene.h"

#include <fstream>
#include <sstream>
#include <json.hpp>

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

	printf("LEVEL CLEANED UP\n");
}

unsigned int fileVersion = 1;
void level::save(std::string level_name) {
	std::string levelPath = "data/scenes/" + level_name;

	std::ofstream ofs(levelPath, std::ofstream::trunc);
	if (ofs.is_open()) {
		ofs << "FILE_VERSION : " << fileVersion << std::endl << std::endl;
		ofs << "PLAYER : " << "Position (" << (int)player::data.pos.x << ", " << (int)player::data.pos.y << ")" << ";" << std::endl;

		ofs << std::endl;

		if (!data.obstacles.empty()) {
			for (auto& _obstacle : data.obstacles) {
				ofs << "OBSTACLE : " << "Position (" << (int)_obstacle->pos.x << ", " << (int)_obstacle->pos.y << ")" << ";" << std::endl;
			}

			ofs << std::endl;
		}

		if (!data.path_nodes.empty()) {
			for (auto& _node : data.path_nodes) {
				ofs << "PATH_NODE : " << "Position (" << (int)_node.x << "," << (int)_node.y << ")" << ";" << std::endl;
			}

			ofs << std::endl;
		}

		if (!data.sprites.empty()) {
			for (auto& _sprite : data.sprites) {
				ofs << "SPRITE : " << "Position (" << (int)_sprite->position->x << ", " << (int)_sprite->position->y << ")" << ";" << std::endl;
			}

			ofs << std::endl;
		}
	}

	ofs.close();
}

void level::load(std::string level_name) {
	clean();
	printf("---------------------\n");
	printf("RETRIEVING SCENE FILE\n");

	std::string levelPath = "data/scenes/" + level_name;

	std::ifstream ifs(levelPath);
	if (ifs.is_open()) {
		printf("PARSING SCENE FILE\n");

		std::string line;
		while (std::getline(ifs, line)) {
			std::stringstream ss(line);
			std::string type;
			
			// TODO: Remove white spaces.
			ss >> std::ws;
			printf("%s\n", ss.str().c_str());
			std::getline(ss, type, ':');

			if (type == "FILE_VERSION") {
				int file_version;
				ss >> file_version;

				if (file_version != fileVersion) {
					printf("OUTDATED SAVE FILE\n");
					return;
				}

				printf("Scene version is %u\n", file_version);
			}
			
			if (type == "PLAYER") {
				glm::ivec2 position;
				ss >> position.x >> position.y;

				player::data.pos = position;
				printf("Player Data Loaded\n");
			} else if (type == "OBSTACLE") {

			}
		}

		printf("FINISHED LOADING SCENE DATA\n");
	}
	else {
		printf("CANNOT FIND SCENE FILE\n");
	}

	ifs.close();

	/*
	nlohmann::json j;
	if (!j["Player"].empty()) {
		for (const auto& data : j["Player"]) {
			player::data.pos.x = data["position.x"];
			player::data.pos.y = data["position.y"];

			printf(" - LOADED PLAYER DATA\n");
		}
	}

	if (!j["Obstacle"].empty()) {
		for (const auto& obstacle_j : j["Obstacle"]) {
			auto o = std::make_shared<obstacle>();

			o->pos.x = obstacle_j["position.x"];
			o->pos.y = obstacle_j["position.y"];

			data.obstacles.push_back(o);
		}
	}
	*/
}