#include "player.h"

#include "input.h"
#include "scene.h"
#include "camera.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>

std::shared_ptr<player_entity> player;

struct player_camera : public camera::camera_interface {
	glm::vec2 offset = { 20,10 };
	player_entity* p;

	player_camera(player_entity* _p) : p(_p){}


	void update(double dt) override {
		glm::vec2 pos = p->pos + ((glm::vec2)p->spr->size * .5f);
		pos += offset;

		glm::ivec2 casted_pos = (glm::ivec2)pos;
		view = glm::translate(glm::mat4(1.0f),
			glm::vec3(casted_pos.x - (renderer::screen_resolution_x / 2),
			casted_pos.y - (renderer::screen_resolution_y / 2),
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
	spr->size = { 16,80 };
	spr->colour = { 0,0,1 };

	col = collision::create_collider();
	col->size = spr->size;

	player_state = PLAYER_STANDING;

	camera::register_camera("Player", std::make_shared<player_camera>(this));
}

player_entity::~player_entity() {
	renderer::delete_sprite(spr);
	collision::delete_collider(col);
}

const float walk_speed = 0.1f;
const float run_speed = 0.5f;
const float aim_speed = 0.025f;
const float slow_speed = 0.05f;
const float fast_speed = 0.75f;

void player_entity::update(double dt) {
	// TODO: implement momentum build up for player.
	// As the player moves their momuntum builds up and
	// makes them go faster until hitting some max speed.
	// Player also doesnt stop instantly, instead their momentum slows down until zero.
	camera::set_camera("Player");

	if (!current_level.path_nodes.empty()) {
		if (input::button_down("MoveLeft")) { player_direction = PLAYER_DIRECTION_LEFT; }
		else if (input::button_down("MoveRight")) { player_direction = PLAYER_DIRECTION_RIGHT; }
		
		if (player_direction == PLAYER_DIRECTION_LEFT) {
			spr->colour = { 1,0,1 };
		}
		else if (player_direction == PLAYER_DIRECTION_RIGHT) {
			spr->colour = { 0,1,1 };
		}

		if(input::button_pressed("MoveLeft") && !input::button_down("MoveLeft")) { vel.x = -1; }
		else if(input::button_pressed("MoveRight") && !input::button_down("MoveRight")) { vel.x = 1; }
		else { vel.x = 0; }

		float movementSpeed = walk_speed;

		if (current_node != current_level.path_nodes.size() - 1 && current_node >= 0) {
			if(current_level.path_nodes[current_node]->is_trigger) {
				// trigger event
			}

			if (current_level.path_nodes[current_node]->flags & PATH_NODE_FAST) {
				movementSpeed = fast_speed;
			}
			else if (current_level.path_nodes[current_node]->flags & PATH_NODE_SLOW) {
				movementSpeed = slow_speed;
			}
			else {
				if (input::button_pressed("Run")) {
					movementSpeed = run_speed;
				}
			}

			if(input::button_released("MoveDown")) {
				player_state = PLAYER_CROUCHED;
			}

			if (current_level.path_nodes[current_node]->flags & PATH_NODE_COMBAT) {
				if (input::button_pressed("Aim")) {
					if (input::button_down("Shoot")) {
						printf("Shoot\n");

						collision::ray_data hit;
						if (collision::check_ray_collision(hit, pos, { 20,0 })) {
							printf("Hit Collider\n");
						}
					}

					movementSpeed = aim_speed;
					return;
				}
			}
		}

		if (vel.x > 0) {
			if (current_node != current_level.path_nodes.size() - 1) {
				if (glm::distance(pos, current_level.path_nodes[current_node + 1]->pos) > 1) {
					glm::vec2 dir = glm::normalize(current_level.path_nodes[current_node + 1]->pos - pos);
					vel = dir * vel.x;

					vel *= dt;
					vel *= movementSpeed;
					
					col->pos = pos + vel;
					if (collision::check_box_collision(col)) {
						return;
					}

					pos += vel;
				}
				else {
					current_node++;
				}
			}
		}
		else {
			if (current_node >= 0) {
				if (glm::distance(pos, current_level.path_nodes[current_node]->pos) > 1) {
					glm::vec2 dir = glm::normalize(current_level.path_nodes[current_node]->pos - pos);
					vel = dir * vel.x;

					vel *= dt;
					vel *= movementSpeed;

					col->pos = pos - vel;
					if (collision::check_box_collision(col)) {
						return;
					}

					pos -= vel;
				}
				else {
					current_node--;
				}
			}
		}
	}
}
