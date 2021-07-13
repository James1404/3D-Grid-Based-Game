#pragma once
#include "../Entity.h"
#include "../SpriteRenderer.h"
#include "../Collision.h"

#include <json.hpp>

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
private:
	SpriteRenderer renderer;
	Collision::Rect collider;
};