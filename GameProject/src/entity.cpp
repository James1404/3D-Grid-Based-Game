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

static float lerp(const float a, const float b, const float t) {
	return (a * (1.0f - t) + (b * t));
}

const float enemy_accel_speed = 0.01f;

const float enemy_walk_speed = 0.025f;
const float enemy_crouch_speed = 0.01f;
const float enemy_staggered_speed = 0.0f;
const float enemy_lunge_speed = 0.05f;
const float enemy_attack_damage = 5;

float enemy_prev_movement_speed;

uint32_t start_stagger_time;
uint32_t max_stagger_time = 1000;

void enemy_entity::update(double dt) {
	if (enemy_state == ENEMY_DEAD)
		return;

	if (start_stagger_time > SDL_GetTicks())
		enemy_state = ENEMY_STAGGERED;
	else
		enemy_state = ENEMY_STANDING;

	float desired_speed = enemy_walk_speed;

	if (enemy_state == ENEMY_CROUCHED)
		desired_speed = enemy_crouch_speed;
	else if (enemy_state == ENEMY_STAGGERED)
		desired_speed = enemy_staggered_speed;

	float movement_speed = lerp(enemy_prev_movement_speed, desired_speed, enemy_accel_speed);

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

		vel *= movement_speed;
		vel *= dt;

		col->pos = pos + vel;
		if (collision::check_box_collision(col))
			return;

		pos += vel;
	}

	col->pos = pos + vel;
	enemy_prev_movement_speed = movement_speed;
}

void enemy_entity::take_damage(int damage_points) {
	current_health_points -= damage_points;
	stagger(); // TODO: enemy is not getting staggered

	if (current_health_points <= 0) {
		enemy_state = ENEMY_DEAD;
		printf("enemy dead\n");
	}
}

void enemy_entity::stagger() {
	// stagger for a certain amount of time then go back
	// to normal state
	printf("Enemy is staggered\n");

	start_stagger_time = SDL_GetTicks() + max_stagger_time;
	enemy_state = ENEMY_STAGGERED;
}
