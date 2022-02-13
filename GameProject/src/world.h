#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm.hpp>

#include "uuid.h"

struct chunk_t;

enum entity_flags 
{
	entity_flags_none = 0,
	entity_flags_disabled = 1 << 0,
	entity_flags_no_collision = 1 << 1,
	entity_flags_no_climb = 1 << 2
};

struct entity_flags_t
{
	entity_flags_t();
	entity_flags_t(uint32_t flags);
	entity_flags_t(const entity_flags_t&) = default;

	operator uint32_t() const { return m_flags; }

	void set(entity_flags mask);
	void clear(entity_flags mask);
	void toggle(entity_flags mask);
	bool has(entity_flags mask);
private:
	uint32_t m_flags = 0;
};

struct entity {
	uuid id = 0;
	int index = 0;

	entity_flags_t flags;
	std::string name = "";

	chunk_t* chunk = nullptr;

	glm::ivec3 grid_pos = glm::vec3(0), previous_grid_pos = glm::vec3(0);
	glm::vec3 visual_pos = glm::vec3(0), visual_rotation = glm::vec3(0), visual_scale = glm::vec3(1);

	virtual void init() {}
	virtual void update(double dt) {}

	bool is_grounded(glm::ivec3 _pos);
	bool is_moving() const;
	void move_grid_pos(glm::ivec3 _dir);
	void set_grid_pos(glm::vec3 _pos);
	void revert_grid_pos();
	void interp_visuals(double dt, float interp_speed);
};

struct entity_data_t
{
	std::string type;
	uuid id;
	entity_flags_t flags;
	glm::ivec3 grid_pos;
};

struct chunk_t
{
	std::string name;

	std::vector<std::shared_ptr<entity>> entities;

	bool check_collisions(glm::vec3 _pos) const;
	bool check_collisions(glm::vec3 _pos, entity* _ignored_entity) const;
	bool check_collisions(glm::vec3 _pos, std::string _tag) const;
	bool check_collisions(glm::vec3 _pos, entity* _ignored_entity, std::string _tag) const;

	std::weak_ptr<entity> get_collisions(glm::vec3 _pos);
	std::weak_ptr<entity> get_collisions(glm::vec3 _pos, entity* _ignored_entity);
	std::weak_ptr<entity> get_collisions(glm::vec3 _pos, std::string _tag);
	std::weak_ptr<entity> get_collisions(glm::vec3 _pos, entity* _ignored_entity, std::string _tag);

	std::vector<glm::ivec3> neighbors(glm::ivec3 _pos) const;
	std::vector<glm::ivec3> diagonal_neighbors(glm::ivec3 _pos) const;

	std::weak_ptr<entity> find_entity_by_id(uuid _id) const;
	std::weak_ptr<entity> find_entity_by_index(int _index) const;
	std::weak_ptr<entity> find_entity_by_position(glm::vec3 _pos) const;

	bool is_entity_at_position(glm::vec3 _pos) const;

	void clear_data();
};

struct world_t
{
	bool is_paused = false;

	std::vector<chunk_t> chunks;
	int current_chunk = 0;

	void init();
	void shutdown();

	chunk_t* get_current_chunk();

	void clear_world_data();
	void update(double dt);

	void save();
	void load();
};

template<typename T>
std::weak_ptr<entity> new_entity(chunk_t& chunk);

void add_entity(chunk_t& chunk, std::string _type, uuid _id, entity_flags_t _flags, glm::ivec3 _grid_pos);
void add_entity(chunk_t& chunk, entity_data_t& data);
void remove_entity(chunk_t& chunk, std::weak_ptr<entity> _entity);

void transition_chunk(chunk_t& next_chunk, entity& entity);
