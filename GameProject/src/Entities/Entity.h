#pragma once
class Entity {
public:
	virtual void init() {}
	virtual void update(double dt) {}
	virtual void render() {}
};