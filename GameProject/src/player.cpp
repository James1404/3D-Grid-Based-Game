#include "player.h"

#include "input.h"
#include "scene.h"
#include "camera.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>
#include <cmath>
#include <SDL.h>

std::shared_ptr<player_entity> player;

static float lerp(const float a, const float b, const float t) {
	return (a * (1.0f - t) + (b * t));
}

static glm::vec2 lerp(const glm::vec2 a, const glm::vec2 b, const float t) {
	return glm::vec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

static glm::vec2 move_towards(const glm::vec2 pos, const glm::vec2 target, const float step) {
	const glm::vec2 delta = target - pos; 		// Gap vector
	const float len2 = glm::dot(delta, delta); 	// Squared length of the gap

	if (len2 < step * step) // were close enought to close the gap in one step
		return target;

	// unit vector that points from 'pos' to 'target'
	const glm::vec2 direction = delta / glm::sqrt(len2);

	// perform the step
	return pos + step * direction;
}

static glm::vec2 vec_floor(glm::vec2 vec) {
	return glm::vec2(floorf(vec.x), floorf(vec.y));
}

static glm::ivec2 vec_to_ivec(glm::vec2 vec) {
	vec = vec_floor(vec);
	return (glm::ivec2)vec;
}

struct player_camera : public camera::camera_interface {
	glm::vec2 offset = { 20,10 };

	glm::vec2 pos = { 0,0 };
	const float camera_speed = 0.01f;
	void update(double dt) override {
		// glm::vec2 pos = (glm::vec2)player->pos * (float)renderer::cell_size + ((float)renderer::cell_size / 2);
		pos = lerp(pos, (glm::vec2)player->previous_pos * (float)renderer::cell_size + ((float)renderer::cell_size / 2), dt * camera_speed);
		// pos = move_towards(pos, (glm::vec2)player->pos * (float)renderer::cell_size + ((float)renderer::cell_size / 2), dt * camera_speed);

		view = glm::translate(glm::mat4(1.0f),
			glm::vec3(pos.x - (renderer::screen_resolution_x / 2),
			pos.y - (renderer::screen_resolution_y / 2),
			0.0f));

		view = glm::inverse(view);
	}
};

player_entity::player_entity() {
	printf("------------------\n");

	spr = renderer::create_sprite();
	//data.spr->set_sprite_path("player.png");
	spr->position = &pos;
	spr->layer = 1;
	spr->colour = { 0,0,1 };

	pos = { 0,0 };
	previous_pos = { 0,0 };
	target_pos = { 0,0 };
	vel = { 0,0 };

	direction = DIRECTION_UP;

	is_dead = false;
	current_health_points = max_health_points;

	camera::register_camera("Player", std::make_shared<player_camera>());
}

player_entity::~player_entity() {
	renderer::delete_sprite(spr);
}

const float movement_speed = 0.005f;
void player_entity::update(double dt) {
	camera::set_camera("Player");

	if(is_dead)
		return;
	
	glm::vec2 new_pos = target_pos;

	/*
	renderer::debug::draw_box_wireframe(pos * glm::ivec2(renderer::cell_size) + glm::ivec2(0, 1) *glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);
	renderer::debug::draw_box_wireframe(pos * glm::ivec2(renderer::cell_size) + glm::ivec2(1, 0) * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);
	renderer::debug::draw_box_wireframe(pos * glm::ivec2(renderer::cell_size) + glm::ivec2(-1, 0) * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);
	renderer::debug::draw_box_wireframe(pos * glm::ivec2(renderer::cell_size) + glm::ivec2(0, -1) * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);
	
	renderer::debug::draw_box_wireframe(pos * glm::ivec2(renderer::cell_size) + glm::ivec2(1, 1) *glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);
	renderer::debug::draw_box_wireframe(pos * glm::ivec2(renderer::cell_size) + glm::ivec2(-1, 1) * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);
	renderer::debug::draw_box_wireframe(pos * glm::ivec2(renderer::cell_size) + glm::ivec2(1, -1) * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);
	renderer::debug::draw_box_wireframe(pos * glm::ivec2(renderer::cell_size) + glm::ivec2(-1, -1) * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);
	*/
	
	/*
	if (vel == glm::vec2(0)) {
		if (input::button_down("MoveUp"))
			new_pos.y++;
		else if (input::button_down("MoveDown"))
			new_pos.y--;

		if (input::button_down("MoveLeft"))
			new_pos.x--;
		else if (input::button_down("MoveRight"))
			new_pos.x++;
	}
	*/

	if (vel.x == 0) {
		if (input::button_pressed("MoveUp") && input::button_pressed("MoveDown"))
			vel.y = 0;
		else if (input::button_pressed("MoveUp") && !input::button_down("MoveUp")) {
			direction = DIRECTION_UP;
			vel.y = 1;
		}
		else if (input::button_pressed("MoveDown") && !input::button_down("MoveDown")) {
			direction = DIRECTION_DOWN;
			vel.y = -1;
		}
		else
			vel.y = 0;

		new_pos.x = floorf(new_pos.x) + 0.5f;
	}

	if (vel.y == 0) {
		if (input::button_pressed("MoveRight") && input::button_pressed("MoveLeft"))
			vel.x = 0;
		else if (input::button_pressed("MoveRight") && !input::button_down("MoveRight")) {
			direction = DIRECTION_RIGHT;
			vel.x = 1;
		}
		else if (input::button_pressed("MoveLeft") && !input::button_down("MoveLeft")) {
			direction = DIRECTION_LEFT;
			vel.x = -1;
		}
		else
			vel.x = 0;

		new_pos.y = floorf(new_pos.y) + 0.5f;
	}

	vel *= movement_speed;
	vel *= dt;

	new_pos += vel;

	renderer::debug::draw_circle(target_pos * glm::vec2(renderer::cell_size), 1, colour::green);

	glm::ivec2 look_dir = { 0,0 };
	if (direction == DIRECTION_UP)
		look_dir = { 0,1 };
	else if (direction == DIRECTION_DOWN)
		look_dir = { 0,-1 };
	else if (direction == DIRECTION_LEFT)
		look_dir = { -1, 0 };
	else if (direction == DIRECTION_RIGHT)
		look_dir = { 1, 0 };

	renderer::debug::draw_box_wireframe(pos * glm::ivec2(renderer::cell_size) + look_dir * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);

	for (auto& obstacle : current_level.obstacles) {
		if (obstacle->pos == vec_to_ivec(new_pos)) {
			return;
		}
	}

	for (auto& enemy : current_level.enemies) {
		if (enemy->pos == vec_to_ivec(new_pos)) {
			return;
		}
	}

	target_pos = new_pos;
	previous_pos = pos;
	pos = vec_to_ivec(target_pos);
}

void player_entity::take_damage(int damage_amount) {
	current_health_points -= damage_amount;

	if (current_health_points <= 0) {
		is_dead = true;
		printf("Player dead\n");
	}
}
