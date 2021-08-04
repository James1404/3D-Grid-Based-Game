#pragma once
#include <stdio.h>
#include <fstream>

#include <json.hpp>

class Entity {
public:
	int id;
	char name[128] = "name";

	Entity() : id(currentID++) { printf("Entity %i Created\n", id); }
	~Entity() { printf("Entity %i Destroyed\n", id); }

	virtual void init() {}
	virtual void update(double dt) {}
	virtual void render() {}

	virtual void editmodeRender() {}

	virtual void SerializeEntity(nlohmann::json& j) { }
private:
	static int currentID;
};