#pragma once
#include <stdio.h>
#include <fstream>

class Entity {
public:
	int id;
	char name[128] = "name";

	Entity() : id(currentID++) { printf("Entity %i Created\n", id); }
	~Entity() { printf("Entity %i Destroyed\n", id); }

	virtual void save(std::ofstream& f) {}
	virtual void load(std::ifstream& f) {}

	virtual void init() {}
	virtual void update(double dt) {}
	virtual void render() {}

	virtual void editmodeRender() {}
private:
	static int currentID;
};