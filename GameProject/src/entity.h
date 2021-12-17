#pragma once
#include <glm.hpp>
#include <gtx/hash.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <SDL.h>

#include "renderer.h"
#include "collision.h"
#include "common.h"

//
// EVENTS
//

struct listener {
	virtual ~listener() {}
	virtual void on_notify() = 0;
};

struct event_manager {
	std::unordered_multimap<std::string, listener*> events;

	void register_listener(std::string _event_name, listener* _listener);
	void remove_listener(std::string _event_name, listener* _listener);
	void notify(std::string _event_name);
	void clear();
};

//
// ENTITY MANAGER
//

struct entity;

struct entity_manager {
	std::string name;

	int step_accumulator = 0;

	event_manager game_event_manager;

	std::vector<std::shared_ptr<entity>> entities;
	std::multimap<std::string, std::shared_ptr<entity>> entities_tag_lookup;

	void init();
	void update(double dt);
	void clean();

	void save();
	void load(std::string level_name);

	// helper functions
	bool is_walkable(glm::ivec2 _pos) const;
	std::vector<glm::ivec2> neighbors(glm::ivec2 _pos) const;
	std::vector<glm::ivec2> diagonal_neighbors(glm::ivec2 _pos) const;

	std::weak_ptr<entity> find_entity_by_tag(std::string _tag) const;

	// collisions
	bool check_collisions(glm::vec2 _pos) const;
	bool check_collisions(glm::vec2 _pos, entity* _ignored_entity) const;
	bool check_collisions(glm::vec2 _pos, std::string _tag) const;
	bool check_collisions(glm::vec2 _pos, entity* _ignored_entity, std::string _tag) const;

	std::weak_ptr<entity> get_collisions(glm::vec2 _pos);
	std::weak_ptr<entity> get_collisions(glm::vec2 _pos, entity* _ignored_entity);
	std::weak_ptr<entity> get_collisions(glm::vec2 _pos, std::string _tag);
	std::weak_ptr<entity> get_collisions(glm::vec2 _pos, entity* _ignored_entity, std::string _tag);

	std::vector<std::weak_ptr<entity>> get_circle_collision(glm::vec2 _pos, float _radius);
	std::vector<std::weak_ptr<entity>> get_circle_collision(glm::vec2 _pos, float _radius, entity* _ignored_entity);
	std::vector<std::weak_ptr<entity>> get_circle_collision(glm::vec2 _pos, float _radius, std::string _tag);
	std::vector<std::weak_ptr<entity>> get_circle_collision(glm::vec2 _pos, float _radius, entity* _ignored_entity, std::string _tag);
};

// 
// Entities
//

typedef uint32_t ENTITY_FLAGS;
enum ENTITY_FLAGS_ {
	ENTITY_NONE = 0,
	ENTITY_DISABLED = 1 << 0,
	ENTITY_NO_DAMAGE = 1 << 1,
	ENTITY_NO_COLLISION = 1 << 2,
	ENTITY_NO_KNOCKBACK = 1 << 3,
	ENTITY_NO_STAGGER = 1 << 4
};
inline void ENTITY_FLAG_SET(ENTITY_FLAGS& x, ENTITY_FLAGS_ mask) { x |= mask; }
inline void ENTITY_FLAG_CLEAR(ENTITY_FLAGS& x, ENTITY_FLAGS_ mask) { x &= ~mask; }
inline void ENTITY_FLAG_TOGGLE(ENTITY_FLAGS& x, ENTITY_FLAGS_ mask) { x ^= mask; }

struct entity {
	ENTITY_FLAGS flags;
	std::string tag;

	entity_manager* manager;

	glm::ivec2 grid_pos, previous_grid_pos;
	glm::vec2 visual_pos;
	glm::vec2 vel;

	int max_health_points = 3;
	int current_health_points;

	bool is_dead = false;
	bool is_moving = false;

	entity()
		: flags(0), tag(""),
		grid_pos(0, 0), previous_grid_pos(0, 0), visual_pos(0, 0), vel(0, 0),
		current_health_points(3), is_dead(false), is_moving(true)
	{
		printf("INITIALIZED ENTITY %p\n", this);
	}

	virtual ~entity() {
		printf("DESTROYED ENTITY %p\n", this);
	}

	virtual void update(double dt) {}

	bool move_grid_pos(glm::ivec2 _dir, int _distance = 1) {
		if (_dir == glm::ivec2(0))
			return false;

		glm::ivec2 new_pos = grid_pos + (_dir * _distance);
		if (!(flags & ENTITY_NO_COLLISION)) {
			if (_distance > 1) {
				for (int i = 0; i < _distance + 1; i++) {
					if (manager->check_collisions(grid_pos + (_distance * i), this)) {
						new_pos = grid_pos + (_distance * (i - 1));
					}
				}
			}
			else {
				if (manager->check_collisions(new_pos, this)) {
					return false;
				}
			}
		}

		previous_grid_pos = grid_pos;
		grid_pos = new_pos;
		return true;
	}

	void move_with_collision(glm::ivec2 _dir, int _distance = 1) {
		glm::ivec2 new_pos = grid_pos + (_dir * _distance);
		if (manager->check_collisions(new_pos, this))
			return;

		previous_grid_pos = grid_pos;
		grid_pos = new_pos;
	}

	void interp_visuals(double dt, float interp_speed) {
		if (vel == glm::vec2(0))
			visual_pos = grid_pos;
		
		visual_pos = common::move_towards(visual_pos, grid_pos, interp_speed * dt);

		is_moving = (visual_pos != (glm::vec2)grid_pos);
	}

	void do_damage(int _damage) {
		if (flags & ENTITY_NO_DAMAGE)
			return;

		if (is_dead)
			return;

		current_health_points -= _damage;

		if (current_health_points <= 0)
			is_dead = true;
	}

	int stagger_end_time = 0;

	void stagger(int _seconds) {
		if (flags & ENTITY_NO_STAGGER)
			return;

		if (is_dead)
			return;

		stagger_end_time = SDL_GetTicks() + (_seconds);
	}

	void knockback(glm::ivec2 _direction, int _range) {
		if (flags & ENTITY_NO_KNOCKBACK)
			return;

		move_grid_pos(_direction, _range);
	}
};