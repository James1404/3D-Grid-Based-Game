#pragma once
#include "../Entity.h"
#include "../Game.h"
#include "../SpriteRenderer.h"
#include "../Collision.h"
#include "../Input.h"

#include <json.hpp>

class Player : public Entity {
public:
	void init() override {
		strcpy_s(name, "Player");

		this->layer = Layers::Player;
		this->renderer.InitSprite("data/textures/player.png");

		this->collider.InitCollider(this);
		this->collider.pos = this->position;
		this->collider.size = { this->renderer.width,this->renderer.height };
	}

	void update(double dt) {
		position.y = 0;

		if (Input::instance().ButtonPressed("MoveLeft"))
		{ this->velocity.x = -1; }
		else if (Input::instance().ButtonPressed("MoveRight"))
		{ this->velocity.x = 1; }
		else
		{ this->velocity.x = 0; }

		float movementSpeed = speed;
		if (Input::instance().ButtonPressed("Run")) { movementSpeed *= .5f; }

		if (Input::instance().ButtonDown("Shoot")) {
			Ray ray(this, position, { 20,0 });
			RayHit hit;

			printf("Shoot\n");
			if (ray.RayVsCollider(hit)) {
				printf("You Hit %s\n", hit.collider->owner->name);
			}
		}

		glm::vec2 moveVector = glm::vec2(std::floor(this->velocity.x), std::floor(this->velocity.y));
		moveVector /= movementSpeed;
		moveVector *= dt;

		this->collider.pos = this->position + moveVector;
		if (collider.ColliderVsCollider()) {
			return;
		}

		this->position += moveVector;
	}

	void render() override {
		this->renderer.DrawSprite((glm::ivec2)this->position, (int)this->layer);
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

	void SerializeEntity(nlohmann::json& j) override {
		j["Player"] += {
			{"name", this->name},
			{ "position.x", this->position.x },
			{ "position.y", this->position.y }
		};
	}

	glm::vec2 position;
	glm::vec2 velocity;
private:
	SpriteRenderer renderer;
	Collider collider;

	float speed = 50;
};