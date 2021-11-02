#include "player.h"

#include "input.h"
#include "scene.h"
#include "camera.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>

std::shared_ptr<player_entity> player;

static float lerp(const float a, const float b, const float t) {
	return (a * (1.0f - t) + (b * t));
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

struct player_camera : public camera::camera_interface {
	glm::vec2 offset = { 20,10 };

	glm::vec2 pos;
	void update(double dt) override {
		// pos = move_towards(pos, player->pos + ((glm::vec2)player->spr->size * .5f) + offset, .125f * dt);
		pos = player->pos + ((glm::vec2)player->spr->size * .5f) + offset;

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
	spr->size = { 16,80 };
	spr->colour = { 0,0,1 };

	col = collision::create_collider();
	col->size = spr->size;

	player_state = PLAYER_STANDING;

	camera::register_camera("Player", std::make_shared<player_camera>());
}

player_entity::~player_entity() {
	renderer::delete_sprite(spr);
	collision::delete_collider(col);
}

const float accel_speed = 0.01f;

const float walk_speed = 0.1f;
const float run_speed = 0.5f;
const float crouch_speed = 0.025f;
const float aim_speed = 0.025f;
const float slow_speed = 0.05f;
const float fast_speed = 0.75f;

float prev_movement_speed;

void player_entity::update(double dt) {
	if(player_state == PLAYER_DEAD)
		return;

	camera::set_camera("Player");

	// change player sprite colour based on direction
	if (player_direction == PLAYER_DIRECTION_LEFT)
		spr->colour = { 1,0,1 };
	else if (player_direction == PLAYER_DIRECTION_RIGHT)
		spr->colour = { 0,1,1 };

	// change collider height based on player state
	if(player_state == PLAYER_STANDING) {
		col->size.y = 80;
		spr->size.y = 80;
	} else if(player_state == PLAYER_CROUCHED) {
		col->size.y = 40;
		spr->size.y = 40;
	}

	// TODO: implement momentum build up for player.
	// As the player moves their momuntum builds up and
	// makes them go faster until hitting some max speed.
	// Player also doesnt stop instantly, instead their momentum slows down until zero.
	if (!current_level.path_nodes.empty()) {
		// change player direction
		if (input::button_down("MoveLeft"))
			player_direction = PLAYER_DIRECTION_LEFT;
		else if (input::button_down("MoveRight"))
			player_direction = PLAYER_DIRECTION_RIGHT;

		// set player velocity
		if(input::button_pressed("MoveLeft") && !input::button_down("MoveLeft")) { vel.x = -1; }
		else if(input::button_pressed("MoveRight") && !input::button_down("MoveRight")) { vel.x = 1; }
		else { vel.x = 0; }

		float desired_speed = walk_speed;

		if (current_node != current_level.path_nodes.size() - 1 && current_node >= 0) {
			if(current_level.path_nodes[current_node]->is_trigger) {
				// trigger event
			}

			// set move speed
			if(player_state == PLAYER_CROUCHED)
				desired_speed = crouch_speed;
			else if (current_level.path_nodes[current_node]->flags & PATH_NODE_FAST)
				desired_speed = fast_speed;
			else if (current_level.path_nodes[current_node]->flags & PATH_NODE_SLOW)
				desired_speed = slow_speed;
			else
				if (input::button_pressed("Run"))
					desired_speed = run_speed;

			// change player state to crouched or standing
			if(input::button_down("MoveDown"))
				player_state = PLAYER_CROUCHED;
			else if(input::button_down("MoveUp"))
				player_state = PLAYER_STANDING;

			if (input::button_pressed("Aim")) {
				if (input::button_down("Shoot")) {
					printf("Shoot\n");

					collision::ray_data hit;
					glm::vec2 ray_origin = { pos.x + (spr->size.x / 2), pos.y + (spr->size.y / 2) };
					float shoot_dir = 100 * ((player_direction == PLAYER_DIRECTION_LEFT) ? -1 : 1);
					for (auto& enemy : current_level.path_nodes[current_node]->enemies) {
						if (collision::line_vs_collider(hit, ray_origin, { shoot_dir,0 }, enemy->col)) {
							printf("Hit Enemy\n");
							enemy->take_damage(1);
						}
					}
				}

				desired_speed = aim_speed;
			}
		}

		float movementSpeed = lerp(prev_movement_speed, desired_speed, accel_speed * dt);

		if (vel.x > 0) {
			if (current_node != current_level.path_nodes.size() - 1) {
				glm::vec2 new_pos = move_towards(pos, current_level.path_nodes[current_node + 1]->pos, movementSpeed * dt);
				
				col->pos = new_pos;
				if (collision::check_box_collision(col))
					return;

				pos = new_pos;
				if(pos == current_level.path_nodes[current_node + 1]->pos)
					current_node++;
			}
		}
		else if(vel.x < 0) {
			if (current_node >= 0) {
				glm::vec2 new_pos = move_towards(pos, current_level.path_nodes[current_node]->pos, movementSpeed * dt);

				col->pos = new_pos;
				if (collision::check_box_collision(col))
					return;

				pos = new_pos;
				if(pos == current_level.path_nodes[current_node]->pos)
					current_node--;
			}
		}

		prev_movement_speed = movementSpeed;
	}
}
