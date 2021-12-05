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
		pos = common::lerp(pos, (glm::vec2)owner->grid_pos * (float)renderer::cell_size + ((float)renderer::cell_size / 2), dt * camera_speed);
	
		view = glm::translate(glm::mat4(1.0f),
			glm::vec3(pos.x - (renderer::screen_resolution_x / 2),
				pos.y - (renderer::screen_resolution_y / 2),
				0.0f));
	
		view = glm::inverse(view);
	}
};

player_entity::player_entity() {
	tag = "player";

	//data.spr->set_sprite_path("player.png");
	spr.position = &visual_pos;
	spr.layer = 1;
	spr.colour = { 0,0,1 };

	vel = { 0,0 };

	direction = { 0,1 };

	is_dead = false;
	current_health_points = max_health_points;

	camera::register_camera("Player", std::make_shared<player_camera>(this));
}

player_entity::~player_entity() {

}

void player_entity::update(double dt) {
	camera::set_camera("Player");

	if (is_dead)
		return;

	renderer::debug::draw_circle(glm::vec2(grid_pos * renderer::cell_size) + ((float)renderer::cell_size / 2), 1, colour::green);
	renderer::debug::draw_box_wireframe(grid_pos * renderer::cell_size + direction * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);
	
	if (vel == glm::vec2(0))
		visual_pos = grid_pos;

	visual_pos = common::move_towards(visual_pos, grid_pos, movement_speed * dt);
	if (visual_pos != (glm::vec2)grid_pos)
		return;

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
	}

	if (input::button_down("Attack")) {
		if (auto hit_entity = manager->get_collisions(grid_pos + direction, "enemy").lock()) {
			hit_entity->do_damage(1);
			printf("Attack\n");
		}
	}

	if (input::button_down("Shoot")) {
		if (shoot_end_time < SDL_GetTicks()) {
			for (int i = 0; i < shoot_range + 1; i++) {
				if (auto hit_entity = manager->get_collisions(grid_pos + (direction * i), "enemy").lock()) {
					hit_entity->do_damage(1);
					hit_entity->knockback(direction, 1);
					break;
				}
			}

			printf("Shoot\n");
			shoot_end_time = SDL_GetTicks() + shoot_cooldown_duration;
		}
	}

	if (manager->check_collisions(grid_pos + (glm::ivec2)vel, this))
		return;

	grid_pos += vel;
}