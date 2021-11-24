#pragma once
#include <glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>

#include "renderer.h"
#include "collision.h"

//
// COMMON FUNCTIONS
//

namespace common {
	inline float lerp(const float a, const float b, const float t) {
		return (a * (1.0f - t) + (b * t));
	}

	inline glm::vec2 lerp(const glm::vec2 a, const glm::vec2 b, const float t) {
		return glm::vec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
	}

	inline glm::vec2 move_towards(const glm::vec2 pos, const glm::vec2 target, const float step) {
		const glm::vec2 delta = target - pos;
		const float len2 = glm::dot(delta, delta);

		if (len2 < step * step)
			return target;

		const glm::vec2 direction = delta / glm::sqrt(len2);

		return pos + step * direction;
	}

	inline glm::vec2 vec_floor(glm::vec2 vec) {
		return glm::vec2(floorf(vec.x), floorf(vec.y));
	}

	inline glm::ivec2 vec_to_ivec(glm::vec2 vec) {
		return (glm::ivec2)vec_floor(vec);
	}

	/*
	inline bool check_collisions(entity_manager* _manager, glm::vec2 _pos, std::string _tag = "", entity* _hit_entity = nullptr) {
		for (const auto& entity : _manager->entities) {
			if (!_tag.empty() && entity->tag != _tag)
				continue;

			if (entity->flags & ENTITY_NO_COLLISION)
				continue;

			if (entity->pos == vec_to_ivec(_pos)) {
				_hit_entity = entity.get();
				return true;
			}
		}

		return false;
	}
	*/
}

struct entity;

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
// EVENT MANAGER
//

struct entity_manager {
	std::string name;

	event_manager game_event_manager;

	std::vector<std::shared_ptr<entity>> entities;
	std::multimap<std::string, entity*> entities_quick_tag_lookup;

	void init();
	void update(double dt);
	void clean();

	void save();
	void load(std::string level_name);

	// collisions
	bool check_collisions(glm::vec2 _pos, std::string _tag = "");
	bool check_collisions(entity* _owner, glm::vec2 _pos, std::string _tag = "");

	entity* get_collisions(glm::vec2 _pos, std::string _tag = "");
	entity* get_collisions(entity* _owner, glm::vec2 _pos, std::string _tag = "");
};

// 
// Entities
//

typedef uint32_t ENTITY_FLAGS;
enum ENTITY_FLAGS_ {
	ENTITY_NONE = 0,
	ENTITY_DISABLED = 1 << 0,
	ENTITY_NO_DAMAGE = 1 << 1,
	ENTITY_NO_COLLISION = 1 << 2
};
inline void ENTITY_FLAG_SET(ENTITY_FLAGS* x, ENTITY_FLAGS_ mask) { *x |= mask; }
inline void ENTITY_FLAG_CLEAR(ENTITY_FLAGS* x, ENTITY_FLAGS_ mask) { *x &= ~mask; }
inline void ENTITY_FLAG_TOGGLE(ENTITY_FLAGS* x, ENTITY_FLAGS_ mask) { *x ^= mask; }

static uint32_t current_id = 0;
struct entity {
	uint32_t id;
	ENTITY_FLAGS flags;
	std::string tag;

	entity_manager* manager;

	glm::ivec2 pos;

	int max_health_points = 3;
	int current_health_points;

	bool is_dead = false;

	entity() : id(current_id++), flags(0), tag(""), pos(0, 0), current_health_points(3), is_dead(false) {
		printf("INITIALIZED ENTITY %p\n", this);
	}

	virtual ~entity() {
		printf("DESTROYED ENTITY %p\n", this);
	}

	virtual void update(double dt) {}

	void do_damage(int _damage) {
		if (flags & ENTITY_NO_DAMAGE)
			return;

		if (is_dead)
			return;

		current_health_points -= _damage;

		if (current_health_points <= 0)
			is_dead = true;
	}

	void knockback(glm::ivec2 _direction, int _range) {
		if (flags & ENTITY_NO_DAMAGE)
			return;

		for (int i = 0; i < _range + 1; i++) {
			if (manager->check_collisions(pos + (_direction * i))) {
				pos += _direction * i;
				return;
			}
		}

		pos += _direction * _range;
	}
};

struct enemy_entity : public entity {
	renderer::sprite* spr;

	enemy_entity();
	~enemy_entity();
	virtual void update(double dt);
};
