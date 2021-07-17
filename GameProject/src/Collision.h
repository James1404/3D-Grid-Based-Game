#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

namespace Collision {
    struct Collider {
        glm::vec2 pos;
        glm::vec2 size;
    };

    struct Ray {
        glm::vec2 origin;
        glm::vec2 direction;
    };

    struct RayHit {
        glm::vec2 point;
        glm::vec2 normal;
        float distance;
    };

    bool ColliderVsCollider(const Collider* collider1, const Collider* collider2);
    bool PointVsCollider(const glm::vec2& point, const Collider* collider);
    bool RayVsCollider(const Ray* ray, const Collider* target, RayHit& hit);
}

class CollisionManager {
public:
    static std::map<int, Collision::Collider*> colliders;

    static void AddCollider(int id, Collision::Collider* collider);
};
