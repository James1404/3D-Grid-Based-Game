#include "SceneSerialization.h"
#include <json.hpp>
#include <fstream>

#include "Entities/Player.h"
#include "Entities/Sprite.h"

void ClearScene(Scene& t_scene) {
	t_scene.entities.clear();
	printf("New Scene\n");
}

void Serialize(Scene& t_scene, const std::string& filePath) {
	nlohmann::json j;
	for (auto const& entity : t_scene.entities) {
		entity->SerializeEntity(j);
	}

	std::ofstream ofs(filePath);
	if (ofs.is_open()) {
		ofs << j.dump(4) << std::endl;
	}
	else {
		printf("Cannot open scene file\n");
	}
}

// TODO: Add Reflection For Deserializeation|
void Deserialize(Scene& t_scene, const std::string& filePath) {
	ClearScene(t_scene);

	nlohmann::json j;
	std::ifstream ifs(filePath);
	if (ifs.is_open()) {
		j = nlohmann::json::parse(ifs);
	}
	else {
		printf("Cannot opne scene file\n");
	}

	ifs.close();

	if (!j["Player"].empty()) {
		for (const auto& player : j["Player"]) {
			auto p = std::make_shared<Player>();
			p->init();
			
			std::string name = player["name"];
			strcpy_s(p->name, name.c_str());

			p->position.x = player["position.x"];
			p->position.y = player["position.y"];

			t_scene.entities.push_back(p);
		}
	}

	if (!j["Player"].empty()) {
		for (const auto& sprite : j["Sprite"]) {
			auto s = std::make_shared<Sprite>();
			s->init();

			std::string name = sprite["name"];
			strcpy_s(s->name, name.c_str());

			s->position.x = sprite["position.x"];
			s->position.y = sprite["position.y"];

			t_scene.entities.push_back(s);
		}
	}
}