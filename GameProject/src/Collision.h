#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

struct Collider {
	glm::vec2 pos;
	glm::vec2 size;

    bool isCollision(const Collider& col);
    glm::vec2 CalculateDistance(const Collider& col);
};

class CollisionManager {
public:
    static std::map<int, Collider> colliders;

    static void AddCollider(int id, glm::vec2 pos, glm::vec2 size);
    static void UpdateCollider(int id, glm::vec2 pos, glm::vec2 size);
};