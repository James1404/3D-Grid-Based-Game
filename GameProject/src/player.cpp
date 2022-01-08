#include "player.h"

#include "input.h"
#include "camera.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>

player_entity::player_entity() {
	tag = "player";

	//data.spr->set_sprite_path("player.png");
	spr.position = &visual_pos;
	spr.colour = { 0,0,1 };

	direction = { 0,1 };

	interp_speed = walk_speed;
}

player_entity::~player_entity() {

}

void player_entity::update(double dt) {
	if (is_dead) {
		spr.colour = { 1, .5f, 0 };
		ENTITY_FLAG_SET(flags, ENTITY_NO_COLLISION);
		return;
	}

	manager->cameras.set_camera("Player");

	if (auto player_cam = manager->cameras.get_camera("Player").lock()) {
		glm::vec3 offset = glm::vec3(0, 5, 5);
		
		glm::vec3 target_pos = ((glm::vec3)grid_pos + offset) * (float)renderer::cell_size + ((float)renderer::cell_size / 2);
		player_cam->position = common::lerp(player_cam->position, target_pos, camera_speed * dt);


		player_cam->rotation.x = 20;
	}

	//renderer::debug::draw_circle(glm::vec2(grid_pos * renderer::cell_size) + ((float)renderer::cell_size / 2), 1, colour::green);
	//renderer::debug::draw_box_wireframe(glm::ivec2(grid_pos) * renderer::cell_size + direction * glm::ivec2(renderer::cell_size), glm::ivec2(renderer::cell_size), colour::pink);

	if (!is_moving) {
		vel = { 0,0,0 };

		if (!(input::button_pressed("MoveUp") && input::button_pressed("MoveDown"))) {
			if (input::button_pressed("MoveUp")) {
				direction = { 0,1 };
				vel = { 0,0,-1 };
			}
			else if (input::button_pressed("MoveDown")) {
				direction = { 0,-1 };
				vel = { 0,0,1 };
			}
		}

		if (!(input::button_pressed("MoveLeft") && input::button_pressed("MoveRight"))) {
			if (input::button_pressed("MoveLeft")) {
				direction = { -1,0 };
				vel = { -1,0,0 };
			}
			else if (input::button_pressed("MoveRight")) {
				direction = { 1,0 };
				vel = { 1,0,0 };
			}
		}

		interp_speed = walk_speed;
		if (input::button_pressed("Run")) {
			interp_speed = run_speed;
		}

		if (input::button_down("Attack")) {
			vel.y = 1;
		}

		if (move_grid_pos(vel))
			state_queue.clear();
		
		switch (state_queue.get()) {
		case player_actions::IDLE:
			break;
		}
	}

	interp_visuals(dt, interp_speed);
}