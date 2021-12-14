#pragma once
#include <glm.hpp>
#include <gtx/hash.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>

#include "renderer.h"
#include "collision.h"

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

	glm::ivec2 grid_pos;
	glm::vec2 visual_pos;

	int max_health_points = 3;
	int current_health_points;

	bool is_dead = false;

	int steps_per_update;

	float visual_interp_speed;

	entity()
		: flags(0), tag(""), grid_pos(0, 0), visual_pos(0, 0),
		current_health_points(3), is_dead(false), steps_per_update(1), visual_interp_speed(0.02f)
	{
		printf("INITIALIZED ENTITY %p\n", this);
	}

	virtual ~entity() {
		printf("DESTROYED ENTITY %p\n", this);
	}

	virtual void update_input(double dt) {}
	virtual void update_logic() {}
	virtual void update_visuals(double dt) {}

	void do_damage(int _damage) {
		if (flags & ENTITY_NO_DAMAGE)
			return;

		if (is_dead)
			return;

		current_health_points -= _damage;

		if (current_health_points <= 0)
			is_dead = true;
	}

	int internal_step_accum = 0;
	int stagger_end_step = 0;

	void stagger(int stagger_duration) {
		if (flags & ENTITY_NO_STAGGER)
			return;

		if (is_dead)
			return;

		stagger_end_step = internal_step_accum + stagger_duration;
	}

	void knockback(glm::ivec2 _direction, int _range) {
		if (flags & ENTITY_NO_KNOCKBACK)
			return;

		for (int i = 0; i < _range + 1; i++) {
			if (manager->check_collisions(grid_pos + (_direction * i), this)) {
				grid_pos += _direction * (i - 1);
				return;
			}
		}

		grid_pos += _direction * _range;
	}
};