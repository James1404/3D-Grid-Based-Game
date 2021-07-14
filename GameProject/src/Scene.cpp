#include "Scene.h"

#include "Entities/Player.h"
#include "Entities/Sprite.h"

#include "SpriteRenderer.h"

#include <json.hpp>
#include <fstream>

#include <iostream>

SpriteRenderer background;

void Scene::newScene() {
	entities.clear();
	printf("New Scene\n");
}

void Scene::saveScene() {
	// Write entity data to json file
	nlohmann::json j;
	for (auto const& entity : entities) {
		entity->to_json(j);
	}

	// Open a new file and write json into it.
	std::ofstream ofs("resources/scenes/Level1.scene");
	if (ofs.is_open()) {
		ofs << j.dump(4) << std::endl;
	}
	else {
		printf("Cannot open file\n");
	}

	ofs.close();
	printf("Save Scene\n");
}

void Scene::loadScene() {
	entities.clear();

	// Write files data to the json variable.
	nlohmann::json j;
	std::ifstream ifs("resources/scenes/Level1.scene");
	if (ifs.is_open()) {
		j = nlohmann::json::parse(ifs);
	}
	else {
		printf("Cannot open file\n");
	}

	ifs.close();
	
	// loop through the json items.
	if (!j["Player"].empty()) {
		for (const auto& player : j["Player"]) {
			auto p = std::make_shared<Player>();
			p->init();
			p->from_json(player);
			entities.push_back(p);
		}
	}

	if (!j["Sprite"].empty()) {
		for (const auto& sprite : j["Sprite"]) {
			auto s = std::make_shared<Sprite>();
			s->init();
			s->from_json(sprite);
			entities.push_back(s);
		}
	}

	printf("Load Scene\n");
}

void Scene::init() {
	background.InitSprite("resources/textures/background.png");

	for (auto entity : entities) {
		entity->init();
	}
}

void Scene::update(double dt) {
	for (auto entity : entities) {
		entity->update(dt);
	}
}

void Scene::render() {
	background.DrawSprite({ 0,0 });

	for (auto entity : entities) {
		entity->render();
	}
}

void Scene::CreateSprite() {
	auto s = std::make_shared<Sprite>();
	s->init();
	entities.push_back(s);
}

void Scene::CreatePlayer() {
	auto p = std::make_shared<Player>();
	p->init();
	entities.push_back(p);
}