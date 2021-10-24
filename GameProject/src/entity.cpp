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

const float enemy_move_speed = 0.1f;
const float enemy_lunge_speed = 0.2f;
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
}

enemy_entity::~enemy_entity() {
	renderer::delete_sprite(spr);
	collision::delete_collider(col);
}

void enemy_entity::update(double dt) {
	if (!current_level.path_nodes.empty()) {
		// if player is at a node behind ours than walk to
		// the previous node and vise versa
		if(player->current_node == current_node) {
			// start chasing player
			float x_dir = glm::normalize(player->pos - pos).x;
			if(x_dir < 0) {
				// player is to the left
			} else if(x_dir > 0){
				// player is to the right
			} else {
				// player is at the same position as enemy
			}
		}
	}

	col->pos = pos + vel;
}
