#include "player.h"

#include "input.h"
#include "camera.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>

struct player_camera : public camera::camera_interface {
	glm::vec2 pos{ 0 };
	const float camera_speed = 0.01f;

	player_entity* owner;

	player_camera(player_entity* _owner) : owner(_owner) { }
	~player_camera() { }

	void update(double dt) override {
		pos = common::lerp(pos, (glm::vec2)owner->grid_pos * (float)renderer::cell_size + ((float)renderer::cell_size / 2), camera_speed * dt);
	
		view = glm::translate(glm::mat4(1.0f),
			glm::vec3(pos.x - (renderer::screen_resolution_x / 2),
			pos.y - (renderer::screen_resolution_y / 2), 0.0f));
	
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

	steps_per_update = 5;

	camera::register_camera("Player", std::make_shared<player_camera>(this));
}

player_entity::~player_entity() {

}

void player_entity::update_input(double dt) {
	if (input::button_down("MoveUp")) {
		state_queue.push(MOVE_UP);
	}
	else if (input::button_down("MoveDown")) {
		state_queue.push(MOVE_DOWN);
	}
	else if (input::button_down("MoveLeft")) {
		state_queue.push(MOVE_LEFT);
	}
	else if (input::button_down("MoveRight")) {
		state_queue.push(MOVE_RIGHT);
	}
	
	if (input::button_pressed("MoveUp") || input::button_pressed("MoveDown")) {
		if (!(input::button_pressed("MoveUp") && input::button_pressed("MoveDown"))) {
			if (input::button_pressed("MoveUp") && !input::button_down("MoveUp")) {
				state_queue.push_at_front(MOVE_UP);
			}
			else if (input::button_pressed("MoveDown") && !input::button_down("MoveDown")) {
				state_queue.push_at_front(MOVE_DOWN);
			}
		}
	}
	else if (input::button_pressed("MoveLeft") || input::button_pressed("MoveRight")) {
		if (!(input::button_pressed("MoveLeft") && input::button_pressed("MoveRight"))) {
			if (input::button_pressed("MoveLeft") && !input::button_down("MoveLeft")) {
				state_queue.push_at_front(MOVE_LEFT);
			}
			else if (input::button_pressed("MoveRight") && !input::button_down("MoveRight")) {
				state_queue.push_at_front(MOVE_RIGHT);
			}
		}
	}

	if (input::button_down("Attack"))
		state_queue.push(ATTACK);

	if (input::button_down("Shoot"))
		state_queue.push(SHOOT);

	if (input::button_pressed("Run") // check if is moving before increasing steps_per_update
		&& (state_queue.front_equals(MOVE_UP)
		|| state_queue.front_equals(MOVE_DOWN)
		|| state_queue.front_equals(MOVE_LEFT)
		|| state_queue.front_equals(MOVE_RIGHT))) {
		steps_per_update = 2;
	}
	else {
		steps_per_update = 4;
	}
}

void player_entity::update_logic() {
	if (is_dead) {
		spr.colour = { 1, .5f, 0 };
		ENTITY_FLAG_SET(flags, ENTITY_NO_COLLISION);
		return;
	}

	switch (state_queue.get()) {
	case player_states::IDLE:
		vel = { 0,0 };
		break;
	case player_states::MOVE_UP:
		direction = { 0,1 };
		vel = { 0,1 };
		break;
	case player_states::MOVE_DOWN:
		direction = { 0,-1 };
		vel = { 0,-1 };
		break;
	case player_states::MOVE_LEFT:
		direction = { -1,0 };
		vel = { -1,0 };
		break;
	case player_states::MOVE_RIGHT:
		direction = { 1,0 };
		vel = { 1,0 };
		break;
	case player_states::ATTACK:
		if (auto hit_entity = manager->get_collisions(grid_pos + direction, "enemy").lock()) {
			hit_entity->do_damage(1);
			printf("Attack\n");
		}
		break;
	case player_states::SHOOT:
		for (int i = 0; i < shoot_range + 1; i++) {
			if (auto hit_entity = manager->get_collisions(grid_pos + (direction * i), "enemy").lock()) {
				hit_entity->do_damage(1);
				hit_entity->knockback(direction, 1);
				break;
			}
		}

		printf("Shoot\n");
		break;
	}

	if (manager->check_collisions(grid_pos + (glm::ivec2)vel, this))
		return;

	grid_pos += vel;
}

void player_entity::update_visuals(double dt) {
	camera::set_camera("Player");

	renderer::debug::draw_circle(glm::vec2(grid_pos * renderer::cell_size) + ((float)renderer::cell_size / 2), 1, colour::green);
	renderer::debug::draw_box_wireframe(grid_pos * renderer::cell_size + direction * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);

	if (vel == glm::vec2(0))
		visual_pos = grid_pos;

	visual_pos = common::move_towards(visual_pos, grid_pos, visual_interp_speed * dt);
}