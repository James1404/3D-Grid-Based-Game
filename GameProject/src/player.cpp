#include "player.h"

#include "input.h"

#include <imgui.h>
#include <json.hpp>

player::player_data player::data;
player::path player::player_path;

float lerpTime = 1.0f;
float currentLerpTime;

glm::vec2 lerp(const glm::vec2& A, const glm::vec2& B, float t) {
	// return A * t + B * (1.f - t);
	return A + (B - A) * t;
}

void player::init() {
	printf("------------------\n");
	data.spr = renderer::create_sprite("data/textures/player.png", &data.pos, 1);
	data.col = collision::create_collider({ data.spr->width, data.spr->height });

	printf("PLAYER INITIALIZED\n");
}

void player::update(double dt) {
	data.pos.y = 0;

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

	currentLerpTime += dt;
	if (currentLerpTime > lerpTime)
		currentLerpTime = lerpTime;

	float perc = currentLerpTime / lerpTime;

	if (perc > 1) {
		data.current_node += 1;
		currentLerpTime = 0;
	}

	data.pos = lerp(data.pos, player_path.nodes[data.current_node + 1], perc);

	/*
	float movementSpeed = data.speed;
	if (input::button_pressed("Run")) { movementSpeed *= .5f; }

	glm::vec2 moveVector = glm::vec2(std::floor(data.vel.x), std::floor(data.vel.y));
	moveVector /= movementSpeed;
	moveVector *= dt;

	data.col->pos = data.pos + moveVector;
	if (collider_vs_collider(data.col)) {
		return;
	}

	data.pos += moveVector;
	*/
}

void player::clean() {
	printf("--------------\n");
	renderer::delete_sprite(data.spr);
	collision::delete_collider(data.col);
	printf("PLAYER CLEANED\n");
}