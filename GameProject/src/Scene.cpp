#include "scene.h"

#include <json.hpp>
#include <fstream>

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

void level::save(std::string level_name) {
	std::string levelPath = "data/scenes/" + level_name;

	nlohmann::json j;

	j["Player"] += {
		{ "position.x", (int)player::data.pos.x },
		{ "position.y", (int)player::data.pos.y }
	};

	for (auto _obstacle : data.obstacles) {
		j["Obstacle"] += {
			{ "position.x", (int)_obstacle->pos.x },
			{ "position.y", (int)_obstacle->pos.y }
		};
	}

	std::ofstream ofs(levelPath);
	if (ofs.is_open()) {
		ofs << j.dump(4) << std::endl;
	}

	ofs.close();
}

void level::load(std::string level_name) {
	clean();
	printf("---------------------\n");
	printf("RETRIEVING SCENE FILE\n");

	std::string levelPath = "data/scenes/" + level_name;

	nlohmann::json j;
	std::ifstream ifs(levelPath);
	if (ifs.is_open()) {
		j = nlohmann::json::parse(ifs);

		printf("PARSING SCENE FILE\n");
	}
	else {
		printf("CANNOT FIND SCENE FILE\n");
	}

	ifs.close();

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

	printf("FINISHED LOADING SCENE DATA\n");
}