#include "player.h"

#include "input.h"
#include "camera.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>

player_entity::player_entity()
	: model("data/models/player.obj", &visual_pos, glm::vec3(0, 0, 1))
{
	tag = "player";

	direction = { 0,1 };

	interp_speed = walk_speed;
}

player_entity::~player_entity() {

}

void player_entity::update(double dt) {
	manager->cameras.set_camera("Player");

	if (auto player_cam = manager->cameras.get_camera("Player").lock()) {
		glm::vec3 offset = glm::vec3(0, 6, 5);
		
		glm::vec3 target_pos = ((glm::vec3)grid_pos + offset);
		player_cam->position = common::lerp(player_cam->position, target_pos, camera_speed * dt);

		player_cam->rotation.x = 20;
	}

	// TODO: if player is not loaded first then they are render after them.
	
	//renderer::debug::draw_box_wireframe(grid_pos + glm::ivec3(direction.x, 0, direction.y), glm::vec3(1), colour::pink);
	
	if (!is_moving()) {
		vel = { 0,0,0 };

		if (!(input::button_pressed("MoveUp") && input::button_pressed("MoveDown"))) {
			if (input::button_pressed("MoveUp")) {
				direction = { 0,-1 };
				vel = { 0,0,-1 };
			}
			else if (input::button_pressed("MoveDown")) {
				direction = { 0,1 };
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

		move_grid_pos(vel);
	}

	interp_visuals(dt, interp_speed);
}