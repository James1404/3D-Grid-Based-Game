#include "Scene.h"

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