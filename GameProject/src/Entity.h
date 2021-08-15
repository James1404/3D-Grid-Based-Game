#pragma once
#include <stdio.h>
#include <fstream>

#include <json.hpp>

enum class Layers {
	Background = -10,
	Foreground = 10,

	StaticCollider = 2,

	Player = 0,
	Enemies = 0,
	
	Interactable = -2
};

// TODO: Implement UUID System.

class Entity {
public:
	int id;
	char name[128] = "name";

	Layers layer;

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