#pragma once
#include "../Entity.h"
#include "../SpriteRenderer.h"
#include "../Collision.h"

#include <json.hpp>

class Sprite : public Entity {
public:
	void init() override {
		strcpy_s(name, "Sprite");

		layer = Layers::StaticCollider;
		renderer.InitSprite("data/textures/face.png");

		collider.InitCollider(this);
		collider.size = { renderer.width,renderer.height };
	}

	void update(double dt) override {
		collider.pos = position;
	}

	void render() override {
		renderer.DrawSprite((glm::ivec2)position, (int)layer);
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
		j["Sprite"] += {
			{"name", name},
			{ "position.x", position.x },
			{ "position.y", position.y }
		};
	}

	glm::vec2 position;
private:
	SpriteRenderer renderer;
	Collider collider;
};