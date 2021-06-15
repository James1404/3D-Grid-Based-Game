#include "Scene.h"

#include "Entities/Player.h"
#include "Entities/Sprite.h"

#include <memory>

std::vector<std::shared_ptr<Entity>> entities;

void Scene::loadScene() {
	entities.push_back(std::make_shared<Player>());

	entities.push_back(std::make_shared<Sprite>());
	entities.push_back(std::make_shared<Sprite>());
	entities.push_back(std::make_shared<Sprite>());
	entities.push_back(std::make_shared<Sprite>());
	entities.push_back(std::make_shared<Sprite>());
	entities.push_back(std::make_shared<Sprite>());
	entities.push_back(std::make_shared<Sprite>());
	entities.push_back(std::make_shared<Sprite>());
	entities.push_back(std::make_shared<Sprite>());
	entities.push_back(std::make_shared<Sprite>());

	this->init();
}

void Scene::init() {
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
	for (auto entity : entities) {
		entity->render();
	}
}
