#include "scene.h"

void scene::update(double dt) {
	for (auto entity : entities) {
		entity->update(dt);
	}
}

void scene::copy_scene_data(scene& _scene) {
	entities.clear();
	for (auto t_entity : _scene.entities) {
		entities.push_back(std::make_shared<entity>(*t_entity));
	}
}