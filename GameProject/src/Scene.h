#pragma once
#include "Entity.h"

#include <vector>
#include <memory>

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