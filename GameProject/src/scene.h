#pragma once
#include "entity.h"
#include "renderer.h"
#include "glm.hpp"

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <unordered_map>

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
// LISTENERS
//

struct cutscene : public listener {
	std::string event_name;

	cutscene();
	~cutscene();

	void update(double dt);

	void on_notify() override;

	bool is_active = false;
};

//
// LEVEL
//

struct level {
	std::string name;

	event_manager game_event_manager;

	std::vector<std::shared_ptr<cutscene>> cutscenes;
	std::vector<std::shared_ptr<enemy_entity>> enemies;
	std::vector<std::shared_ptr<obstacle_entity>> obstacles;

	std::vector<std::shared_ptr<sprite_entity>> sprites;

	void init();
	void update(double dt);
	void clean();

	void save();
	void load(std::string level_name);
};

extern level current_level;
