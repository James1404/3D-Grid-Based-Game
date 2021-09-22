#pragma once
#include "entity.h"
#include "renderer.h"
#include "collision.h"

#include <imgui.h>
#include <json.hpp>

struct obstacle : public entity {
	glm::vec2 pos;
	renderer::sprite* spr;
	collider col;

	obstacle() {
		strcpy_s(name, "Obstacle");

		spr = renderer::create_sprite("data/textures/face.png", &pos, -1);

		col.init_collider(this);
		col.size = { spr->width,spr->height };
	}

	~obstacle() {
		renderer::delete_sprite(spr);
	}

	void update(double dt) override {
		col.pos = pos;
	}

#ifdef _DEBUG
	void editor_draw() override {
		ImGui::PushID(id);
		ImGui::InputText("Name", name, IM_ARRAYSIZE(name));
		ImGui::Text("ID: %i", id);

		ImGui::Separator();

		ImGui::DragFloat("Position.x", &pos.x);
		ImGui::DragFloat("Position.y", &pos.y);
		ImGui::PopID();
	}
#endif // _DEBUG

	void serialize_entity(nlohmann::json& j) override {
		j["Obstacle"] += {
			{"name", name},
			{ "position.x", pos.x },
			{ "position.y", pos.y }
		};
	}

};