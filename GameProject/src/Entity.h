#pragma once
#include <stdio.h>
#include <fstream>

#include <json.hpp>

// TODO: Implement UUID System.
static unsigned int currentID = 0;

struct entity {
	unsigned int id;

	entity() : id(currentID++) {}
	~entity() {}

	virtual void update(double dt) {}
};