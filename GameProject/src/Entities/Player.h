#pragma once
#include "../Entity.h"
#include "../Game.h"
#include "../SpriteRenderer.h"
#include "../Collision.h"

#include <json.hpp>

class Player : public Entity {
public:
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
		
		glm::vec2 moveVector = glm::vec2(std::floor(this->velocity.x), std::floor(this->velocity.y));
		moveVector /= speed;

		this->collider.pos = this->position + moveVector;

		CollisionManager::UpdateCollider(id, &this->collider);

		for (const auto& collider : CollisionManager::colliders) {
			if (collider.first != id) {
				if (Collision::RectVsRect(&this->collider, collider.second)) {
					this->collider.pos = glm::vec2(this->position.x + moveVector.x, this->position.y);
					if (Collision::RectVsRect(&this->collider, collider.second)) {
						this->collider.pos = glm::vec2(this->position.x, this->position.y + moveVector.y);
						if (Collision::RectVsRect(&this->collider, collider.second)) {
							return;
						}
						else {
							moveVector.x = 0;
						}
					}
					else {
						moveVector.y = 0;
					}
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

	nlohmann::json to_json() override {
		return nlohmann::json{
			{"name", this->name},
			{"position.x", this->position.x},
			{"position.y", this->position.y}
		};
	}

	void from_json(nlohmann::json& j) {
		j.at("name").get_to(this->name);
		j.at("position.x").get_to(this->position.x);
		j.at("position.y").get_to(this->position.y);
	}

	glm::vec2 position;
	glm::vec2 velocity;
private:
	SpriteRenderer renderer;
	Collision::Rect collider;

	float speed = 50;
};