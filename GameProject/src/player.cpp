#include "player.h"

#include "input.h"

#include <imgui.h>
#include <json.hpp>

#include <cmath>

player::player_data player::data;
player::path player::player_path;

float distance(const glm::vec2 a, const glm::vec2 b) {
	float x_diff = a.x - b.x;
	float y_diff = a.y - b.y;
	return std::sqrt(x_diff * x_diff + y_diff * y_diff);
}

void player::init() {
	printf("------------------\n");
	data.spr = renderer::create_sprite("data/textures/player.png", &data.pos, 1);
	data.col = collision::create_collider({ data.spr->width, data.spr->height });

	printf("PLAYER INITIALIZED\n");
}

int current_node = 0;
void player::update(double dt) {
	if (input::button_pressed("Aim")) {
		if (input::button_down("Shoot")) {
			printf("Shoot\n");

			collision::ray_data hit;
			if (collision::ray_vs_collider(hit, data.pos, { 20,0 })) {
				printf("Hit Collider\n");
			}
		}

		return;
	}

	if (input::button_pressed("MoveLeft")) { data.vel.x = -1; }
	else if (input::button_pressed("MoveRight")) { data.vel.x = 1; }
	else { data.vel.x = 0; }

	float movementSpeed = .1f;
	if (input::button_pressed("Run")) { movementSpeed = .5f; }

	if (data.vel.x > 0) {
		if (current_node != player_path.nodes.size() - 1) {
			if (distance(data.pos, player_path.nodes[current_node + 1]) > 0.1) {
				glm::vec2 dir = glm::normalize(player_path.nodes[current_node + 1] - data.pos);
				glm::vec2 move_vector = dir * data.vel.x;

				move_vector *= dt;
				move_vector *= movementSpeed;

				data.col->pos = data.pos + move_vector;
				if (collider_vs_collider(data.col)) {
					return;
				}

				data.pos += move_vector;
			}
			else {
				current_node++;
			}
		}
	}
	else {
		if (current_node >= 0) {
			if (distance(data.pos, player_path.nodes[current_node]) > 0.01) {
				glm::vec2 dir = glm::normalize(player_path.nodes[current_node] - data.pos);
				glm::vec2 move_vector = dir * data.vel.x;

				move_vector *= dt;
				move_vector *= movementSpeed;

				data.col->pos = data.pos - move_vector;
				if (collider_vs_collider(data.col)) {
					return;
				}

				data.pos -= move_vector;
			}
			else {
				current_node--;
			}
		}
	}
}

void player::clean() {
	printf("--------------\n");
	renderer::delete_sprite(data.spr);
	collision::delete_collider(data.col);
	printf("PLAYER CLEANED\n");
}