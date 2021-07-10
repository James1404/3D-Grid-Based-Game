#include "Scene.h"

#include "Entities/Player.h"
#include "Entities/Sprite.h"

#include "SpriteRenderer.h"

SpriteRenderer background;

void Scene::newScene() {
	entities.clear();
}

void Scene::saveScene() {
	std::ofstream fs;
	fs.open("Level1.bin", std::ios::out | std::ios::binary | std::ios::app);
	if (fs.is_open()) {
		for (auto entity : entities) {
			fs.write((char*)&entity, sizeof(entity));
		}
	}
	else {
		printf("Cannot open file\n");
	}

	fs.close();
}

void Scene::loadScene() {
	entities.clear();

	std::ifstream binaryFile("Level1.bin", std::ios::binary);

	Entity entity;
	while (binaryFile.read(reinterpret_cast<char*>(&entity), sizeof(entity))) {
		auto e = std::make_shared<Entity>(entity);
		e->init();
		entities.push_back(e);
	}

	binaryFile.close();
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