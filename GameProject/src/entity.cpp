#include "entity.h"

#include "scene.h"
#include "player.h"

obstacle_entity::obstacle_entity() {
	spr = renderer::create_sprite();
	spr->position = &pos;
	spr->layer = -1;
	spr->size = { 48,48 };
	spr->colour = colour::green;

	col = collision::create_collider();
	col->size = spr->size;
}

obstacle_entity::~obstacle_entity() {
	renderer::delete_sprite(spr);
	collision::delete_collider(col);
}

void obstacle_entity::update(double dt) {
	col->pos = pos;
}

sprite_entity::sprite_entity() {
	// sprite_path = "";

	spr = renderer::create_sprite();
	spr->position = &pos;
	spr->layer = 0;
	//spr->set_sprite_path(sprite_path.c_str());
	spr->size = { 1,1 };
	spr->colour = { 0,0,0 };
}

sprite_entity::~sprite_entity() {
	renderer::delete_sprite(spr);
}

void sprite_entity::update(double dt) {

}

const float enemy_move_speed = 0.025f;
const float enemy_lunge_speed = 0.05f;
const float enemy_attack_damage = 5;

enemy_entity::enemy_entity() {
	vel = { 0,0 };

	spr = renderer::create_sprite();
	spr->position = &pos;
	spr->layer = -1;
	//spr->set_sprite_path("player.png");
	spr->size = { 16,80 };
	spr->colour = { 1,0,0 };

	col = collision::create_collider();
	col->size = spr->size;

	current_health_points = max_health_points;
}

enemy_entity::~enemy_entity() {
	renderer::delete_sprite(spr);
	collision::delete_collider(col);
}

void enemy_entity::update(double dt) {
	// ENEMY IS DEAD
	if (current_health_points <= 0)
		return;

	if (!current_level.path_nodes.empty()) {
		// start chasing player
		if (player->pos.x > pos.x)
			enemy_direction = ENEMY_DIRECTION_RIGHT;
		else
			enemy_direction = ENEMY_DIRECTION_LEFT;

		spr->colour = (enemy_direction == ENEMY_DIRECTION_LEFT) ? colour::green : colour::red;

		if (enemy_direction == ENEMY_DIRECTION_LEFT)
			vel.x = -1;
		else if (enemy_direction == ENEMY_DIRECTION_RIGHT)
			vel.x = 1;

		vel *= enemy_move_speed;
		vel *= dt;

		col->pos = pos + vel;
		if (collision::check_box_collision(col))
			return;

		pos += vel;
	}

	col->pos = pos + vel;
}

void enemy_entity::take_damage(int damage_points) {
	current_health_points -= damage_points;
}

void enemy_entity::stagger(int stagger_time) {
	// stagger for a certain amount of time then go back
	// to normal state
	printf("Enemy is staggered\n");
}
