#pragma once
#include "Entities/Entity.h"

#include <vector>
#include <memory>
#include <fstream>

class Scene {
public:
	std::vector<std::shared_ptr<Entity>> entities;

	void newScene();
	void saveScene();
	void loadScene();

	void init();
	void update(double dt);
	void render();

	void CreateSprite();
	void CreatePlayer();
};