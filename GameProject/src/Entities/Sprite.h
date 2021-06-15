#pragma once
#include "Entity.h"
#include "../SpriteRenderer.h"
#include "../Game.h"

class Sprite : public Entity {
public:
	void init() override {
		this->position = glm::vec2(rand() % Game::Width, rand() % Game::Height);

		this->renderer.InitSprite();
	}

	void update(double dt) override {

	}

	void render() override {
		this->renderer.DrawSprite(this->position);
	}
private:
	SpriteRenderer renderer;

	glm::vec2 position;
};