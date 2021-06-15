#pragma once
#include "Entity.h"
#include "../Game.h"

class EditmodeCamera : public Entity {
public:
	void update(double dt) override {
		if (Game::event.type == SDL_KEYDOWN) {
			switch (Game::event.key.keysym.sym) {
			case SDLK_w:
				this->velocity.y = -speed;
				break;
			case SDLK_s:
				this->velocity.y = speed;
				break;
			case SDLK_a:
				this->velocity.x = speed;
				break;
			case SDLK_d:
				this->velocity.x = -speed;
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

		Game::view = glm::translate(Game::view, glm::vec3(velocity, 0.0f));
	}
private:
	glm::vec2 velocity;
	float speed = 1.0f;
};