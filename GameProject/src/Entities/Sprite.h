#pragma once
#include "Entity.h"
#include "../SpriteRenderer.h"
#include "../Game.h"
#include "../Collision.h"

class Sprite : public Entity {
public:
	void init() override {
		this->position = glm::vec2(rand() % Game::Width, rand() % Game::Height);
		this->renderer.InitSprite("resources/textures/face.png");

		collider.pos = this->position;
		collider.size = { this->renderer.width,this->renderer.height };

		CollisionManager::AddCollider(id, &this->collider);
	}

	void update(double dt) override {
		CollisionManager::UpdateCollider(id, &this->collider);
	}

	void render() override {
		this->renderer.DrawSprite(this->position);
	}
private:
	SpriteRenderer renderer;
	Rect collider;

	glm::vec2 position;
};