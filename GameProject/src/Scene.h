#pragma once
#include "Entities/Entity.h"

#include <vector>

class Scene {
public:
	void loadScene();
	void init();
	void update(double dt);
	void render();
};