#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <cassert>

struct listener
{
	virtual ~listener() {}
	virtual void on_notify() = 0;
};

struct event_manager_t
{
	std::unordered_multimap<std::string, listener*> events;

	void register_listener(std::string _event_name, listener* _listener);
	void remove_listener(std::string _event_name, listener* _listener);
	void notify(std::string _event_name);

	void init();
	void shutdown();

	static event_manager_t& get()
	{
		static event_manager_t* instance = NULL;
		if(instance == NULL)
		{
			instance = new event_manager_t;
		}
		assert(instance);
		return *instance;
	}
};
