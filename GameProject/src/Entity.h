#pragma once
#include <stdio.h>
#include <fstream>

#include <json.hpp>

// TODO: Implement UUID System.
static int currentID = 0;

struct entity {
	int id;
	char name[128] = "name";

	entity() : id(currentID++) { printf("Entity %i Created\n", id); }
	~entity() { printf("Entity %i Destroyed\n", id); }

	virtual void update(double dt) {}

#ifdef _DEBUG
	virtual void editor_draw() {}
#endif // _DEBUG

	virtual void serialize_entity(nlohmann::json& j) { }
};