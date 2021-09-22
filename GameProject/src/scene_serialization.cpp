#include "scene_serialization.h"
#include <json.hpp>
#include <fstream>

#include "player.h"
#include "obstacle.h"

void clear_scene(scene& t_scene) {
	t_scene.entities.clear();
	printf("New Scene\n");
}

void save_scene(scene& t_scene, const std::string& filePath) {
	nlohmann::json j;
	for (auto const& entity : t_scene.entities) {
		entity->serialize_entity(j);
	}

	std::ofstream ofs(filePath);
	if (ofs.is_open()) {
		ofs << j.dump(4) << std::endl;
	}
}

// TODO: Add Reflection For Deserializeation
void load_scene(scene& t_scene, const std::string& filePath) {
	clear_scene(t_scene);

	nlohmann::json j;
	std::ifstream ifs(filePath);
	if (ifs.is_open()) {
		j = nlohmann::json::parse(ifs);
	}
	else {
		printf("CANNOT FIND SCENE FILE\n");
	}

	ifs.close();

	if (!j["Player"].empty()) {
		for (const auto& _player : j["Player"]) {
			auto p = std::make_shared<player>();
			
			std::string name = _player["name"];
			strcpy_s(p->name, name.c_str());

			p->pos.x = _player["position.x"];
			p->pos.y = _player["position.y"];

			t_scene.entities.push_back(p);
		}
	}

	if (!j["Obstacle"].empty()) {
		for (const auto& _obstacle : j["Obstacle"]) {
			auto o = std::make_shared<obstacle>();

			std::string name = _obstacle["name"];
			strcpy_s(o->name, name.c_str());

			o->pos.x = _obstacle["position.x"];
			o->pos.y = _obstacle["position.y"];

			t_scene.entities.push_back(o);
		}
	}

	printf("FINISHED LOADING SCENE\n");
}