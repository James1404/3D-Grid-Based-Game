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
#include "common.h"
#include "log.h"
#include "camera.h"

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
	camera_manager cameras;

	std::vector<std::shared_ptr<entity>> entities;
	std::multimap<std::string, std::shared_ptr<entity>> entities_tag_lookup;

	void init();
	void update(double dt);
	void clean();

	void save();
	void load(std::string level_name);

	// helper functions
	std::vector<glm::ivec3> neighbors(glm::ivec3 _pos) const;
	std::vector<glm::ivec3> diagonal_neighbors(glm::ivec3 _pos) const;

	std::weak_ptr<entity> find_entity_by_tag(std::string _tag) const;

	// collisions
	bool check_collisions(glm::vec3 _pos) const;
	bool check_collisions(glm::vec3 _pos, entity* _ignored_entity) const;
	bool check_collisions(glm::vec3 _pos, std::string _tag) const;
	bool check_collisions(glm::vec3 _pos, entity* _ignored_entity, std::string _tag) const;

	std::weak_ptr<entity> get_collisions(glm::vec3 _pos);
	std::weak_ptr<entity> get_collisions(glm::vec3 _pos, entity* _ignored_entity);
	std::weak_ptr<entity> get_collisions(glm::vec3 _pos, std::string _tag);
	std::weak_ptr<entity> get_collisions(glm::vec3 _pos, entity* _ignored_entity, std::string _tag);
};

// 
// Entities
//

typedef uint32_t ENTITY_FLAGS;
enum ENTITY_FLAGS_ {
	ENTITY_NONE = 0,
	ENTITY_DISABLED = 1 << 0,
	ENTITY_NO_COLLISION = 1 << 1,
	ENTITY_NO_CLIMB = 1 << 2
};
inline void ENTITY_FLAG_SET(ENTITY_FLAGS& x, ENTITY_FLAGS_ mask) { x |= mask; }
inline void ENTITY_FLAG_CLEAR(ENTITY_FLAGS& x, ENTITY_FLAGS_ mask) { x &= ~mask; }
inline void ENTITY_FLAG_TOGGLE(ENTITY_FLAGS& x, ENTITY_FLAGS_ mask) { x ^= mask; }

struct entity {
	ENTITY_FLAGS flags;
	std::string tag;

	entity_manager* manager;

	glm::ivec3 grid_pos, previous_grid_pos;
	glm::vec3 visual_pos;
	glm::vec3 vel;

	int max_health_points = 3;
	int current_health_points;

	bool is_dead = false;

	entity()
		: flags(0), tag(""),
		grid_pos(0, 0, 0), previous_grid_pos(0, 0, 0), visual_pos(0, 0, 0), vel(0, 0, 0),
		current_health_points(3), is_dead(false)
	{
		logger::info("INITIALIZED ENTITY ", this);
	}

	virtual ~entity() {
		logger::info("DESTROYED ENTITY ", this);
	}

	virtual void update(double dt) {}

	bool is_grounded(glm::ivec3 _pos) {
		return manager->check_collisions(_pos + glm::ivec3(0, -1, 0), this);
	}

	bool is_moving() const {
		return (visual_pos != (glm::vec3)grid_pos);
	}

	void move_grid_pos(glm::ivec3 _dir) {
		if (is_moving())
			return;

		glm::ivec3 new_pos = grid_pos + _dir;
		if (is_grounded(grid_pos)) {
			if (!manager->check_collisions(new_pos, this)) {
				if (is_grounded(new_pos)) {
					set_grid_pos(new_pos);
				}
				else {
					if (!manager->check_collisions(new_pos + glm::ivec3(0, -1, 0))) {
						if (is_grounded(new_pos + glm::ivec3(0, -1, 0)))
							set_grid_pos(new_pos + glm::ivec3(0, -1, 0));
					}
				}
			}
			else {
				if (!manager->check_collisions(new_pos + glm::ivec3(0, 1, 0))) {
					set_grid_pos(new_pos + glm::ivec3(0, 1, 0));
				}
			}
		}
	}

	void set_grid_pos(glm::vec3 _pos) {
		previous_grid_pos = grid_pos;
		grid_pos = _pos;
	}

	void revert_grid_pos() {
		grid_pos = previous_grid_pos;
	}

	void interp_visuals(double dt, float interp_speed) {
		visual_pos = common::move_towards(visual_pos, grid_pos, interp_speed * dt);
	}
};