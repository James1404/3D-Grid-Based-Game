#pragma once
#include "Entity.h"
#include "Sprite.h"

#include "../Game.h"
#include "../SpriteRenderer.h"
#include "../Collision.h"

class Player : public Entity {
public:
	void init() override {
		this->renderer.InitSprite();

		CollisionManager::AddCollider("playerCollider", this->position, { 100,100 });
	}

	void update(double dt) {
		if (Game::event.type == SDL_KEYDOWN) {
			switch (Game::event.key.keysym.sym) {
			case SDLK_w:
				this->velocity.y = speed;
				break;
			case SDLK_s:
				this->velocity.y = -speed;
				break;
			case SDLK_a:
				this->velocity.x = -speed;
				break;
			case SDLK_d:
				this->velocity.x = speed;
				break;
			default:
				break;
			}
		}

		if (Game::event.type == SDL_KEYUP) {
			switch (Game::event.key.keysym.sym) {
			case SDLK_w:
				this->velocity.y = 0;
				break;
			case SDLK_s:
				this->velocity.y = 0;
				break;
			case SDLK_a:
				this->velocity.x = 0;
				break;
			case SDLK_d:
				this->velocity.x = 0;
				break;
			default:
				break;
			}
		}

		CollisionManager::UpdateCollider("playerCollider", this->position + this->velocity, { 100,100 });

		for (auto collider : CollisionManager::colliders) {
			if (collider.first != "playerCollider") {
				if (CollisionManager::colliders["playerCollider"].isCollision(collider.second)) {
					velocity *= -1;
					printf("isColliding");
				}
			}
		}

		this->position.x += this->velocity.x * dt;
		this->position.y += this->velocity.y * dt;
	}

	void render() override {
		this->renderer.DrawSprite(position);
	}

	glm::vec2 position;
	glm::vec2 velocity;
private:
	SpriteRenderer renderer;

	float speed = 1.0f;
};