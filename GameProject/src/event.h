#pragma once
#include <string>
#include <unordered_map>
#include <map>

//
// EVENTS
//

struct listener {
	virtual ~listener() {}
	virtual void on_notify() = 0;
};

struct event_manager_t {
	std::unordered_multimap<std::string, listener*> events;

	event_manager_t();
	~event_manager_t();

	void register_listener(std::string _event_name, listener* _listener);
	void remove_listener(std::string _event_name, listener* _listener);
	void notify(std::string _event_name);
};
