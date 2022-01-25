#include "event.h"

//
// EVENT MANAGER
//

event_manager_t::event_manager_t()
{}

event_manager_t::~event_manager_t()
{
	events.clear();
}

void event_manager_t::register_listener(std::string _event_name, listener* _listener)
{
	events.emplace(_event_name, _listener);
}

void event_manager_t::remove_listener(std::string _event_name, listener* _listener)
{
	auto range = events.equal_range(_event_name);
	for (auto i = range.first; i != range.second; ++i)
	{
		if (i->second == _listener)
		{
			events.erase(i);
			break;
		}
	}
}

void event_manager_t::notify(std::string _event_name)
{
	auto range = events.equal_range(_event_name);
	for (auto i = range.first; i != range.second; ++i)
	{
		i->second->on_notify();
	}
}
