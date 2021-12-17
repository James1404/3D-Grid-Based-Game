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

	direction = { 0,1 };

	interp_speed = walk_speed;

	camera::register_camera("Player", std::make_shared<player_camera>(this));
}

player_entity::~player_entity() {

}

void player_entity::update(double dt) {
	if (is_dead) {
		spr.colour = { 1, .5f, 0 };
		ENTITY_FLAG_SET(flags, ENTITY_NO_COLLISION);
		return;
	}

	if (input::button_down("Shoot"))
		state_queue.push(SHOOT);

	if (input::button_down("Attack"))
		state_queue.push(ATTACK);

	// visuals
	camera::set_camera("Player");

	renderer::debug::draw_circle(glm::vec2(grid_pos * renderer::cell_size) + ((float)renderer::cell_size / 2), 1, colour::green);
	renderer::debug::draw_box_wireframe(grid_pos * renderer::cell_size + direction * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);

	if (!is_moving) {
		vel = { 0,0 };

		if (!(input::button_pressed("MoveUp") && input::button_pressed("MoveDown"))) {
			if (input::button_pressed("MoveUp")) {
				direction = { 0,1 };
				vel = { 0,1 };
			}
			else if (input::button_pressed("MoveDown")) {
				direction = { 0,-1 };
				vel = { 0,-1 };
			}
		}

		if (!(input::button_pressed("MoveLeft") && input::button_pressed("MoveRight"))) {
			if (input::button_pressed("MoveLeft")) {
				direction = { -1,0 };
				vel = { -1,0 };
			}
			else if (input::button_pressed("MoveRight")) {
				direction = { 1,0 };
				vel = { 1,0 };
			}
		}

		if (input::button_pressed("Run")) {
			interp_speed = run_speed;
		}

		if (move_grid_pos(vel))
			state_queue.clear();

		switch (state_queue.get()) {
		case player_actions::IDLE:
			break;
		case player_actions::ATTACK:
			if (auto hit_entity = manager->get_collisions(grid_pos + direction, "enemy").lock()) {
				hit_entity->do_damage(1);
				hit_entity->stagger(2);
			}

			printf("Attack\n");
			break;
		case player_actions::SHOOT:
			for (int i = 0; i < shoot_range + 1; i++) {
				if (auto hit_entity = manager->get_collisions(grid_pos + (direction * i), "enemy").lock()) {
					hit_entity->do_damage(1);
					hit_entity->knockback(direction, 1);
					hit_entity->stagger(5);
					break;
				}
			}

			printf("Shoot\n");
			break;
		}
	}

	interp_visuals(dt, interp_speed);
}