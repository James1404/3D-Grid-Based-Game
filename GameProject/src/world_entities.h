#pragma once
#include "common.h"
#include "entity.h"
#include "pathfinding.h"

//
// ENEMIES
//
struct pusher_enemy_entity : public entity {
	renderer::sprite spr;

	glm::ivec2 direction;

	glm::ivec2 player_path_position;
	int current_path_waypoint = 0;
	std::vector<glm::ivec2> path;

	float visual_interp_speed = 0.002f;

	pusher_enemy_entity() {
		tag = "enemy";

		spr.position = &visual_pos;
		spr.layer = -1;
		spr.colour = { 1,0,0 };

		current_path_waypoint = 0;
	}

	~pusher_enemy_entity() {

	}

	void update(double dt) override {
		if (is_dead) {
			spr.colour = { 1, .5f, 0 };
			ENTITY_FLAG_SET(flags, ENTITY_NO_COLLISION);
			return;
		}

		// visuals
		for (auto neighbour : manager->neighbors(grid_pos)) {
			renderer::debug::draw_box_wireframe(neighbour * renderer::cell_size, glm::vec2(renderer::cell_size), colour::purple);
		}

		// logic
		if (!is_moving) {
			if (auto player_ref = manager->find_entity_by_tag("player").lock()) {
				if (player_ref->grid_pos != player_path_position) {
					std::unordered_map<glm::ivec2, glm::ivec2> came_from;
					std::unordered_map<glm::ivec2, int> cost_so_far;

					a_star_search(*manager, grid_pos, player_ref->grid_pos, came_from, cost_so_far);
					path = reconstruct_path(grid_pos, player_ref->grid_pos, came_from);

					player_path_position = player_ref->grid_pos;
					current_path_waypoint = 0;
				}
			}

			if (path.empty()) {
				vel = { 0,0 };
				return;
			}

			if (grid_pos == path[current_path_waypoint] && current_path_waypoint + 1 < path.size())
				current_path_waypoint++;

			vel = path[current_path_waypoint] - grid_pos;
			vel = glm::normalize(vel);

			for (auto neighbour : manager->neighbors(grid_pos)) {
				if (auto hit_player = manager->get_collisions(neighbour, "player").lock()) {
					glm::ivec2 dir = glm::normalize((glm::vec2)(hit_player->grid_pos - grid_pos));
					//hit_player->knockback(dir, 3);
					//hit_player->stagger(2);
					//printf("push\n");
					return;
				}
			}

			move_grid_pos(vel);
		}

		interp_visuals(dt, visual_interp_speed);
	}
};
