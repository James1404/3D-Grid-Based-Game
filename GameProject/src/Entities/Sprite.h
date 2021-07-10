#pragma once
#include "Entity.h"
#include "../SpriteRenderer.h"
#include "../Game.h"
#include "../Collision.h"

class Sprite : public Entity {
public:
	void init() override {
		strcpy_s(name, "Sprite");

		this->renderer.InitSprite("resources/textures/face.png");

		this->collider.pos = this->position;
		this->collider.size = { this->renderer.width,this->renderer.height };

		CollisionManager::AddCollider(id, &this->collider);
	}

	void update(double dt) override {
		this->collider.pos = this->position;

		CollisionManager::UpdateCollider(id, &this->collider);
	}

	void render() override {
		this->renderer.DrawSprite(this->position);
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
private:
	SpriteRenderer renderer;
	Collision::Rect collider;

	glm::vec2 position;
};