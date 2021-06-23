#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

struct Rect {
	glm::vec2 pos;
	glm::vec2 size;
};

struct Ray {
    glm::vec2 origin;
    glm::vec2 direction;
};

class Collision {
public:
    static bool RectVsRect(const Rect* rect1, const Rect* rect2);
    static bool PointVsRect(const glm::vec2* point, const Rect* rect);
};

class CollisionManager {
public:
    static std::map<int, Rect*> colliders;

    static void AddCollider(int id, Rect* rect);
    static void UpdateCollider(int id, Rect* rect);
};