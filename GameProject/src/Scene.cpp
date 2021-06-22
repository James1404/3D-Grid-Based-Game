#include "Scene.h"

#include "Entities/Player.h"
#include "Entities/Sprite.h"

#include "SpriteRenderer.h"

#include <memory>

std::vector<std::shared_ptr<Entity>> entities;
SpriteRenderer background;

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

	background.InitSprite("resources/textures/background.png");

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
	background.DrawSprite({ 0,0 });

	for (auto entity : entities) {
		entity->render();
	}
}
