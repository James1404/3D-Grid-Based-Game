#pragma once
#include <stdio.h>
#include <fstream>

#include <json.hpp>

// TODO: Implement UUID System.
static unsigned int currentID = 0;

struct entity {
	unsigned int id;

	entity() : id(currentID++) { printf("Entity %i Created\n", id); }
	~entity() { printf("Entity %i Destroyed\n", id); }

	virtual void update(double dt) {}
};