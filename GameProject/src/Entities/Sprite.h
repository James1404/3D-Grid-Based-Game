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

	void to_json(nlohmann::json& j) override {
		j["Sprite"] += {
			{"name", this->name},
			{ "position.x", this->position.x },
			{ "position.y", this->position.y }
		};
	}

	void from_json(const nlohmann::json& j) override {
		std::string name = j["name"];
		strcpy_s(this->name, name.c_str());

		this->position.x = j["position.x"];
		this->position.y = j["position.y"];
	}

	glm::vec2 position;
private:
	SpriteRenderer renderer;
	Collision::Collider collider;
};