#include "player.h"

#include "input.h"
#include "camera.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>
#include <cmath>
#include <SDL.h>

struct player_camera : public camera::camera_interface {
	glm::vec2 offset = { 20,10 };

	glm::vec2 pos = { 0,0 };
	const float camera_speed = 0.01f;

	player_entity* owner;

	player_camera(player_entity* _owner) : owner(_owner) { }
	~player_camera() { }

	void update(double dt) override {
		pos = common::lerp(pos, (glm::vec2)owner->previous_pos * (float)renderer::cell_size + ((float)renderer::cell_size / 2), dt * camera_speed);
	
		view = glm::translate(glm::mat4(1.0f),
			glm::vec3(pos.x - (renderer::screen_resolution_x / 2),
				pos.y - (renderer::screen_resolution_y / 2),
				0.0f));
	
		view = glm::inverse(view);
	}
};

player_entity::player_entity() {
	printf("------------------\n");

	tag = "player";

	spr = renderer::create_sprite();
	//data.spr->set_sprite_path("player.png");
	spr->position = &pos;
	spr->layer = 1;
	spr->colour = { 0,0,1 };

	pos = { 0,0 };
	previous_pos = { 0,0 };
	target_pos = { 0,0 };
	vel = { 0,0 };

	direction = { 0,1 };

	is_dead = false;
	current_health_points = max_health_points;

	camera::register_camera("Player", std::make_shared<player_camera>(this));
}

player_entity::~player_entity() {
	renderer::delete_sprite(spr);
}

const int shoot_range = 5;
const float movement_speed = 0.005f;
void player_entity::update(double dt) {
	camera::set_camera("Player");

	if (is_dead)
		return;

	glm::vec2 new_pos = target_pos;

	if (vel.x == 0) {
		if (input::button_pressed("MoveUp") && input::button_pressed("MoveDown"))
			vel.y = 0;
		else if (input::button_pressed("MoveUp") && !input::button_down("MoveUp")) {
			direction = { 0,1 };
			vel.y = 1;
		}
		else if (input::button_pressed("MoveDown") && !input::button_down("MoveDown")) {
			direction = { 0,-1 };
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
			direction = { 1,0 };
			vel.x = 1;
		}
		else if (input::button_pressed("MoveLeft") && !input::button_down("MoveLeft")) {
			direction = { -1,0 };
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
	renderer::debug::draw_box_wireframe(pos * glm::ivec2(renderer::cell_size) + direction * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);

	if (input::button_down("Attack")) {
		entity* hit_entity = manager->get_collisions(pos + direction, "enemy");
		if (hit_entity != nullptr) {
			printf("Attack\n");
		}
	}

	if (input::button_down("Shoot")) {
		for (int i = 0; i < shoot_range + 1; i++) {
			entity* hit_entity = manager->get_collisions(pos + (direction * i), "enemy");
			if (hit_entity != nullptr) {
				printf("Hit enemy\n");
				hit_entity->do_damage(1);
				hit_entity->knockback(direction, 1);
				break;
			}
		}
	}

	if (manager->check_collisions(this, new_pos)) {
		return;
	}

	target_pos = new_pos;
	previous_pos = pos;
	pos = common::vec_to_ivec(target_pos);
}