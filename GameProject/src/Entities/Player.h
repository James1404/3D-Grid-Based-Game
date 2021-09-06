#pragma once
#include "../Entity.h"
#include "../Game.h"
#include "../SpriteRenderer.h"
#include "../Collision.h"
#include "../Input.h"

#include <json.hpp>

class Player : public Entity {
public:
	enum class PlayerState {
		STATE_IDLE,
		STATE_MOVING,
		STATE_AIMING
	} CurrentState;

	void init() override {
		strcpy_s(name, "Player");

		layer = Layers::Player;
		renderer.InitSprite("data/textures/player.png");

		collider.InitCollider(this);
		collider.pos = position;
		collider.size = { renderer.width,renderer.height };
	}

	void update(double dt) {
		position.y = 0;

		switch (CurrentState) {
			case PlayerState::STATE_IDLE: {
				if (ButtonDown("MoveLeft") || ButtonDown("MoveRight")) { CurrentState = PlayerState::STATE_MOVING; }
				if (ButtonDown("Aim")) { CurrentState = PlayerState::STATE_AIMING; }
				break;
			}
			case PlayerState::STATE_MOVING: {
				if (ButtonPressed("MoveLeft")) { velocity.x = -1; }
				else if (ButtonPressed("MoveRight")) { velocity.x = 1; }
				else { CurrentState = PlayerState::STATE_IDLE; }

				float movementSpeed = speed;
				if (ButtonPressed("Run")) { movementSpeed *= .5f; }

				glm::vec2 moveVector = glm::vec2(std::floor(velocity.x), std::floor(velocity.y));
				moveVector /= movementSpeed;
				moveVector *= dt;

				collider.pos = position + moveVector;
				if (collider.ColliderVsCollider()) {
					return;
				}

				this->position += moveVector;
				break;
			}
			case PlayerState::STATE_AIMING: {
				if (ButtonDown("Shoot")) {
					printf("Shoot\n");

					RayHit hit;
					if (RayVsCollider(this, hit, position, { 20,0 })) {
						printf("You Hit %s\n", hit.collider->owner->name);
					}
				}

				if (ButtonReleased("Aim")) {
					CurrentState = PlayerState::STATE_IDLE;
				}
				break;
			}
		}
	}

	void render() override {
		renderer.DrawSprite((glm::ivec2)this->position, (int)layer);
	}

	void editmodeRender() override {
		ImGui::PushID(id);
		ImGui::InputText("Name", name, IM_ARRAYSIZE(name));
		ImGui::Text("ID: %i", id);

		ImGui::Separator();

		ImGui::DragFloat("Position.x", &position.x);
		ImGui::DragFloat("Position.y", &position.y);
		ImGui::PopID();
	}

	void SerializeEntity(nlohmann::json& j) override {
		j["Player"] += {
			{"name", name},
			{ "position.x", position.x },
			{ "position.y", position.y }
		};
	}

	glm::vec2 position;
	glm::vec2 velocity;
private:
	SpriteRenderer renderer;
	Collider collider;

	float speed = 50;
};