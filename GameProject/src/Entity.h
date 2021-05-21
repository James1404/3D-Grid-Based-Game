#pragma once
#include "Vector2.h"

class Entity {
public:
	Vector2 position;

	virtual void init() {}
	virtual void update() {}
	virtual void render() {}
};