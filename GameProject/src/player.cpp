#include "player.h"

#include "input.h"
#include "scene.h"
#include "camera.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>

struct player_camera : public camera::camera_interface {
	glm::vec2 offset = { 20,10 };

	void update(double dt) override {
		glm::vec2 pos = player::data.pos + ((glm::vec2)player::data.spr->size * .5f);
		pos += offset;

		glm::ivec2 casted_pos = (glm::ivec2)pos;
		view = glm::translate(glm::mat4(1.0f),
			glm::vec3(casted_pos.x - (renderer::screen_resolution_x / 2),
			casted_pos.y - (renderer::screen_resolution_y / 2),
			0.0f));

		view = glm::inverse(view);
	}
};

player::player_data player::data;
void player::init() {
	printf("------------------\n");

	data.current_node = 0;
	data.pos = { 0,0 };

	data.spr = renderer::create_sprite();
	//data.spr->set_sprite_path("player.png");
	data.spr->position = &data.pos;
	data.spr->layer = 1;
	data.spr->size = { 16,80 };
	data.spr->colour = { 0,0,1 };

	data.col = collision::create_collider(data.spr->size);

	printf("PLAYER INITIALIZED\n");

	camera::register_camera("Player", std::make_shared<player_camera>());
}

const float walk_speed = 0.1f;
const float run_speed = 0.5f;
const float aim_speed = 0.025f;
const float slow_speed = 0.05f;
const float fast_speed = 0.75f;

void player::update(double dt) {
	// TODO: implement momentum build up for player.
	// As the player moves their momuntum builds up and
	// makes them go faster until hitting some max speed.
	// Player also doesnt stop instantly, instead their momentum slows down until zero.
	camera::set_camera("Player");

	if (!level::data.path_nodes.empty()) {	
		if (input::button_pressed("MoveLeft")) { data.vel.x = -1; }
		else if (input::button_pressed("MoveRight")) { data.vel.x = 1; }
		else { data.vel.x = 0; }
		
		float movementSpeed = walk_speed;

		if (data.current_node != level::data.path_nodes.size() - 1 && data.current_node >= 0) {
			if (level::data.path_nodes[data.current_node]->flags & PATH_NODE_FAST) {
				movementSpeed = fast_speed;
			}
			else if (level::data.path_nodes[data.current_node]->flags & PATH_NODE_SLOW) {
				movementSpeed = slow_speed;
			}
			else {
				if (input::button_pressed("Run")) {
					movementSpeed = run_speed;
				}
			}

			if (level::data.path_nodes[data.current_node]->flags & PATH_NODE_COMBAT) {
				if (input::button_pressed("Aim")) {
					if (input::button_down("Shoot")) {
						printf("Shoot\n");

						collision::ray_data hit;
						if (collision::check_ray_collision(hit, data.pos, { 20,0 })) {
							printf("Hit Collider\n");
						}
					}

					movementSpeed = aim_speed;
				}
			}
		}

		printf("Current_node %i\n", data.current_node);
		if (data.vel.x > 0) {
			if (data.current_node != level::data.path_nodes.size() - 1) {
				if (glm::distance(data.pos, level::data.path_nodes[data.current_node + 1]->pos) > 1) {
					glm::vec2 dir = glm::normalize(level::data.path_nodes[data.current_node + 1]->pos - data.pos);
					data.vel = dir * data.vel.x;

					data.vel *= dt;
					data.vel *= movementSpeed;
					
					data.col->pos = data.pos + data.vel;
					if (collision::check_box_collision(data.col)) {
						return;
					}

					data.pos += data.vel;
				}
				else {
					data.current_node++;
				}
			}
		}
		else {
			if (data.current_node >= 0) {
				if (glm::distance(data.pos, level::data.path_nodes[data.current_node]->pos) > 1) {
					glm::vec2 dir = glm::normalize(level::data.path_nodes[data.current_node]->pos - data.pos);
					data.vel = dir * data.vel.x;

					data.vel *= dt;
					data.vel *= movementSpeed;

					data.col->pos = data.pos - data.vel;
					if (collision::check_box_collision(data.col)) {
						return;
					}

					data.pos -= data.vel;
				}
				else {
					data.current_node--;
				}
			}
		}
			/*
		if (data.vel.x > 0) {
			if (data.current_node != level::data.path_nodes.size() - 1) {
				if (glm::distance(data.pos, level::data.path_nodes[data.current_node + 1]->pos) > 1) {
					glm::vec2 dir = glm::normalize(level::data.path_nodes[data.current_node + 1]->pos - data.pos);
					data.vel = dir * data.vel.x;

					data.vel *= dt;
					data.vel *= movementSpeed;

					data.col->pos = data.pos + data.vel;
					if (collider_vs_collider(data.col)) {
						return;
					}

					data.pos += data.vel;
				}
				else {
					data.current_node++;
				}
			}
		}
		else {
			if (data.current_node >= 0) {
				if (glm::distance(data.pos, level::data.path_nodes[data.current_node]->pos) > 1) {
					glm::vec2 dir = glm::normalize(level::data.path_nodes[data.current_node]->pos - data.pos);
					data.vel = dir * data.vel.x;

					data.vel *= dt;
					data.vel *= movementSpeed;

					data.col->pos = data.pos - data.vel;
					if (collider_vs_collider(data.col)) {
						return;
					}

					data.pos -= data.vel;
				}
				else {
					data.current_node--;
				}
			}
		}
		*/
	}
}

void player::clean() {
	printf("--------------\n");
	renderer::delete_sprite(data.spr);
	collision::delete_collider(data.col);
	printf("PLAYER CLEANED\n");
}
