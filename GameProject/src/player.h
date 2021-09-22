#pragma once
#include "entity.h"
#include "renderer.h"
#include "collision.h"
#include "Input.h"

#include <imgui.h>
#include <json.hpp>

struct player : public entity {
	glm::vec2 pos;
	glm::vec2 vel;
	renderer::sprite* spr;
	collider* col;
	float speed = 50;
	player() {
		strcpy_s(name, "Player");

		spr = renderer::create_sprite("data/textures/player.png", &pos, 1);
		col = create_collider(this, { spr->width, spr->height });
	}

	~player() {
		renderer::delete_sprite(spr);
		delete_collider(col);
	}

	void update(double dt) {
		pos.y = 0;

		if (input::button_pressed("Aim")) {
			if (input::button_down("Shoot")) {
				printf("Shoot\n");

				ray_data hit;
				if (ray_vs_collider(this, hit, pos, { 20,0 })) {
					printf("You Hit %s\n", hit.col->owner->name);
				}
			}

			return;
		}

		if (input::button_pressed("MoveLeft")) { vel.x = -1; }
		else if (input::button_pressed("MoveRight")) { vel.x = 1; }
		else { vel.x = 0; }

		float movementSpeed = speed;
		if (input::button_pressed("Run")) { movementSpeed *= .5f; }

		glm::vec2 moveVector = glm::vec2(std::floor(vel.x), std::floor(vel.y));
		moveVector /= movementSpeed;
		moveVector *= dt;

		col->pos = pos + moveVector;
		if (collider_vs_collider(col)) {
			return;
		}

		this->pos += moveVector;
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
		j["Player"] += {
			{"name", name},
			{ "position.x", pos.x },
			{ "position.y", pos.y }
		};
	}
};