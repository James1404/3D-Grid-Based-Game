#pragma once
#include "../Entity.h"
#include "../Game.h"
#include "../SpriteRenderer.h"

class Player : public Entity {
public:
	Player() { }

	void init() override {
		renderer = new SpriteRenderer();
		renderer->InitSprite();
	}

	void update() override {
		if (Game::event.type == SDL_KEYDOWN) {
			switch (Game::event.key.keysym.sym) {
			case SDLK_w:
				this->velocity.y = -speed;
				break;
			case SDLK_s:
				this->velocity.y = speed;
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

		this->position.x += this->velocity.x * Game::deltaTime;
		this->position.y += this->velocity.y * Game::deltaTime;
	}

	void render() override {
		renderer->DrawSprite(position);
	}
private:
	SpriteRenderer* renderer;

	glm::vec2 position;
	glm::vec2 velocity;
	float speed = 0.5f;
};