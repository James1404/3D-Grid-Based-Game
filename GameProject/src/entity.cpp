#include "entity.h"

#include "scene.h"
#include "player.h"

obstacle_entity::obstacle_entity() {
	spr = renderer::create_sprite();
	spr->position = &pos;
	spr->layer = -1;
	spr->colour = colour::green;
}

obstacle_entity::~obstacle_entity() {
	renderer::delete_sprite(spr);
}

void obstacle_entity::update(double dt) {
}

sprite_entity::sprite_entity() {
	// sprite_path = "";

	spr = renderer::create_sprite();
	spr->position = &pos;
	spr->layer = 0;
	//spr->set_sprite_path(sprite_path.c_str());
	spr->colour = { 0,0,0 };
}

sprite_entity::~sprite_entity() {
	renderer::delete_sprite(spr);
}

void sprite_entity::update(double dt) {

}

enemy_entity::enemy_entity() {
	spr = renderer::create_sprite();
	spr->position = &pos;
	spr->layer = -1;
	//spr->set_sprite_path("player.png");
	spr->colour = { 1,0,0 };

	is_dead = false;

	current_health_points = max_health_points;
}

enemy_entity::~enemy_entity() {
	renderer::delete_sprite(spr);
}

void enemy_entity::update(double dt) {
	if (is_dead)
		return;
}