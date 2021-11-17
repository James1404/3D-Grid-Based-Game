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

struct player_camera : public camera::camera_interface {
	glm::vec2 offset = { 20,10 };

	void update(double dt) override {
		glm::vec2 pos = (glm::vec2)player->pos * (float)renderer::cell_size + ((float)renderer::cell_size / 2);

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
	target_pos = { 0,0 };
	vel = { 0,0 };

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

	if (vel.x == 0) {
		if (input::button_pressed("MoveUp") && input::button_pressed("MoveDown"))
			vel.y = 0;
		else if (input::button_pressed("MoveUp") && !input::button_down("MoveUp"))
			vel.y = 1;
		else if (input::button_pressed("MoveDown") && !input::button_down("MoveDown"))
			vel.y = -1;
		else {
			vel.y = 0;
			new_pos.y = (int)new_pos.y + 0.5f;
		}
	}

	if (vel.y == 0) {
		if (input::button_pressed("MoveRight") && input::button_pressed("MoveLeft"))
			vel.x = 0;
		else if (input::button_pressed("MoveRight") && !input::button_down("MoveRight"))
			vel.x = 1;
		else if (input::button_pressed("MoveLeft") && !input::button_down("MoveLeft"))
			vel.x = -1;
		else {
			vel.x = 0;
			new_pos.x = (int)new_pos.x + 0.5f;
		}
	}

	vel *= movement_speed;
	vel *= dt;

	new_pos += vel;

	printf("%f %f;\n", new_pos.x, new_pos.y);

	renderer::debug::draw_circle(target_pos * glm::vec2(renderer::cell_size), renderer::cell_size, colour::green);

	for (auto& obstacle : current_level.obstacles) {
		if (obstacle->pos == (glm::ivec2)new_pos) {
			return;
		}
	}

	for (auto& enemy : current_level.enemies) {
		if (enemy->pos == (glm::ivec2)new_pos) {
			return;
		}
	}

	target_pos = new_pos;
	pos = target_pos;

}

void player_entity::take_damage(int damage_amount) {
	current_health_points -= damage_amount;

	if (current_health_points <= 0) {
		is_dead = true;
		printf("Player dead\n");
	}
}
