#pragma once
#include "Entities/Entity.h"

#include <vector>
#include <memory>
#include <fstream>

class Scene {
public:
	void saveScene();
	void loadScene();
	void init();
	void update(double dt);
	void render();

	void CreateSprite();
	void CreatePlayer();
};