#pragma once
class Entity {
public:
	int id;

	virtual void init() {}
	virtual void update(double dt) {}
	virtual void render() {}
private:
	static int currentID;
protected:
	Entity() : id(currentID++) { }
	~Entity() { }
};