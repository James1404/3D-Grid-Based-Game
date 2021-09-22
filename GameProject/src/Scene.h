#pragma once
#include "entity.h"

#include <vector>
#include <memory>

struct scene {
	std::vector<std::shared_ptr<entity>> entities;

	void update(double dt);
	void copy_scene_data(scene& _scene);
};