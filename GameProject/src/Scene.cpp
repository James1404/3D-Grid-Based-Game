#include "Scene.h"

#include "Entities/Player.h"
#include "Entities/Sprite.h"

#include "SpriteRenderer.h"

#include <json.hpp>
#include <fstream>

SpriteRenderer background;

void Scene::newScene() {
	entities.clear();
	printf("New Scene\n");
}

void Scene::saveScene() {
	// Write entity data to json file
	nlohmann::json j;
	for (auto const& entity : entities) {
		j += entity->to_json();
	}

	// Open a new file and write json into it.
	std::ofstream ofs("Level1.json");
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
	std::ifstream ifs("Level1.json");
	if (ifs.is_open()) {
		ifs >> j;
	}
	else {
		printf("Cannot open file\n");
	}

	ifs.close();
	
	// loop through the json items.
	for (const auto& item : j.items()) {

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