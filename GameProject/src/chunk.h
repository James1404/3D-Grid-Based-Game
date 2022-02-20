#pragma once
#include <string>
#include <memory>
#include <vector>
#include <glm.hpp>

#include "asset.h"
#include "uuid.h"
#include "renderer.h"

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

struct entity_data_t
{
	std::string type;
	uuid id;
	entity_flags_t flags;
	glm::ivec3 grid_pos;
	transform_t visual_transform;

	entity_data_t()
		: type(""), id(0), flags(0), grid_pos(0), visual_transform(transform_t())
	{}

	entity_data_t(std::string type, uuid id, entity_flags_t flags, glm::ivec3 grid_pos)
		: type(type), id(id), flags(flags), grid_pos(grid_pos), visual_transform(transform_t(grid_pos, glm::vec3(0), glm::vec3(1)))
	{}
};

struct entity_t {
	uuid id = 0;
	int index = 0;

	entity_flags_t flags;
	std::string name = "";

	std::shared_ptr<chunk_t> chunk;

	glm::ivec3 grid_pos = glm::vec3(0), previous_grid_pos = glm::vec3(0);
	transform_t visual_transform;

	virtual void init() {}
	virtual void update(double dt) {}

	entity_data_t extract_data();

	bool is_grounded(glm::ivec3 _pos);
	bool is_moving() const;
	void set_grid_pos(glm::vec3 _pos);
	void revert_grid_pos();
	void interp_visuals(double dt, float interp_speed);
};


struct node_t
{
	std::string type;

	glm::ivec3 pos;

	virtual void init();
	virtual void update(double dt);
};

struct volume_t
{
	std::string type;

	std::string event;
	glm::ivec3 pos, size;

	virtual void init();
	virtual void update(double dt);
};

struct chunk_t
{
	std::string name;

	std::vector<std::shared_ptr<entity_t>> entities;
	std::vector<std::shared_ptr<node_t>> nodes;
	std::vector<std::shared_ptr<volume_t>> volumes;

	bool check_collisions(glm::vec3 _pos) const;
	bool check_collisions(glm::vec3 _pos, entity_t* _ignored_entity) const;
	bool check_collisions(glm::vec3 _pos, std::string _tag) const;
	bool check_collisions(glm::vec3 _pos, entity_t* _ignored_entity, std::string _tag) const;

	std::weak_ptr<entity_t> get_collisions(glm::vec3 _pos);
	std::weak_ptr<entity_t> get_collisions(glm::vec3 _pos, entity_t* _ignored_entity);
	std::weak_ptr<entity_t> get_collisions(glm::vec3 _pos, std::string _tag);
	std::weak_ptr<entity_t> get_collisions(glm::vec3 _pos, entity_t* _ignored_entity, std::string _tag);

	std::vector<glm::ivec3> neighbors(glm::ivec3 _pos) const;
	std::vector<glm::ivec3> diagonal_neighbors(glm::ivec3 _pos) const;

	std::weak_ptr<entity_t> find_entity_by_id(uuid _id) const;
	std::weak_ptr<entity_t> find_entity_by_index(int _index) const;
	std::weak_ptr<entity_t> find_entity_by_position(glm::vec3 _pos) const;

	bool is_entity_at_position(glm::vec3 _pos) const;
	std::weak_ptr<entity_t> get_entity_at_position(glm::vec3 _pos);

	void clear_data();
};
