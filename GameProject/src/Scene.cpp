#include "Scene.h"

#include "Entities/Player.h"
#include "Entities/Sprite.h"

#include "SpriteRenderer.h"

#include <json.hpp>
#include <fstream>

#include <iostream>

SpriteRenderer background;

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