#pragma once
#include <glm.hpp>
#include <gtx/hash.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <queue>

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

	template<typename T, int Size>
	class Limited_Queue {
	public:
		void push(T _item) {
			if (size() >= Size)
				return;

			queue.push(_item);
		}

		void push_only_to_front(T _item) {
			if (size() != 0)
				return;

			queue.push(_item);
		}

		T get() {
			if (empty())
				return T();

			T _item = queue.front();
			queue.pop();
			return _item;
		}

		T front() const {
			if (empty())
				return T();

			return queue.front();
		}

		bool front_equals(T _item) const {
			if (empty())
				return false;

			return front() == _item;
		}

		bool empty() const { return queue.empty(); }
		int size() const { return queue.size(); }
	private:
		std::queue<T> queue;
	};
}

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

struct entity;

struct entity_manager {
	std::string name;

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
	virtual void update_logic(int steps) {}
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

	void knockback(glm::ivec2 _direction, int _range) {
		if (flags & ENTITY_NO_DAMAGE)
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

struct enemy_entity : public entity {
	renderer::sprite spr;

	glm::vec2 vel;
	glm::ivec2 direction;

	glm::ivec2 player_path_position;
	int current_path_waypoint = 0;
	std::vector<glm::ivec2> path;

	enemy_entity();
	~enemy_entity();
	void update_input(double dt) override;
	void update_logic(int steps) override;
	void update_visuals(double dt) override;
};
