#pragma once
#include "Entity.h"
#include "Sprite.h"

#include "../Game.h"
#include "../SpriteRenderer.h"
#include "../Collision.h"

class Player : public Entity {
public:
	void init() override {
		this->renderer.InitSprite("resources/textures/player.png");
		this->collider.pos = this->position;
		this->collider.size = { this->renderer.width,this->renderer.height };
		CollisionManager::AddCollider(id, &this->collider);
	}

	void update(double dt) {
		if (Game::event.type == SDL_KEYDOWN) {
			switch (Game::event.key.keysym.sym) {
			case SDLK_w:
				this->velocity.y = 1;
				break;
			case SDLK_s:
				this->velocity.y = -1;
				break;
			case SDLK_a:
				this->velocity.x = -1;
				break;
			case SDLK_d:
				this->velocity.x = 1;
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
		
		glm::normalize(velocity);
		velocity *= speed;

		this->collider.pos = this->position + this->velocity;

		CollisionManager::UpdateCollider(id, &this->collider);

		for (const auto& collider : CollisionManager::colliders) {
			if (collider.first != id) {
				if (Collision::RectVsRect(&this->collider, collider.second)) {
					return;
				}
			}
		}

		this->position.x += this->velocity.x * dt;
		this->position.y += this->velocity.y * dt;
	}

	void render() override {
		this->renderer.DrawSprite(static_cast<glm::ivec2>(position));
	}

	glm::vec2 position;
	glm::vec2 velocity;
private:
	SpriteRenderer renderer;
	Rect collider;

	float speed = 1.0f;
};