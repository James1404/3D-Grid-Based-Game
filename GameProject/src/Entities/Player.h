#pragma once
#include "Entity.h"
#include "Sprite.h"

#include "../Game.h"
#include "../SpriteRenderer.h"
#include "../Collision.h"

class Player : public Entity {
public:
	void save(std::ofstream& f) override {
		f.write((char*)&position, sizeof(position));
	}

	void load(std::ifstream& f) override {
		f.read((char*)&position, sizeof(position));
	}

	void init() override {
		strcpy_s(name, "Player");

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
		
		glm::vec2 moveVector = glm::vec2(std::floor(velocity.x), std::floor(velocity.y));
		moveVector /= speed;
		moveVector *= dt;

		this->collider.pos = this->position + moveVector;

		CollisionManager::UpdateCollider(id, &this->collider);

		for (const auto& collider : CollisionManager::colliders) {
			if (collider.first != id) {
				if (Collision::RectVsRect(&this->collider, collider.second)) {
					return;
				}
			}
		}

		this->position += moveVector;
	}

	void render() override {
		this->renderer.DrawSprite(static_cast<glm::ivec2>(position));
	}

	void editmodeRender() override {
		ImGui::PushID(this->id);
		ImGui::InputText("Name", this->name, IM_ARRAYSIZE(this->name));
		ImGui::Text("ID: %i", this->id);

		ImGui::Separator();

		ImGui::DragFloat("Position.x", &this->position.x);
		ImGui::DragFloat("Position.y", &this->position.y);
		ImGui::PopID();
	}

	glm::vec2 position;
	glm::vec2 velocity;
private:
	SpriteRenderer renderer;
	Rect collider;

	float speed = 10;
};