#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

namespace Collision {
    struct Collider {
        Collider();
        ~Collider();

        unsigned int id;

        glm::vec2 pos;
        glm::vec2 size;

        bool ColliderVsCollider();
    private:
        static unsigned int currentID;
    };

    struct RayHit {
        glm::vec2 point;
        glm::vec2 normal;
        float distance;
    };

    struct Ray {
        glm::vec2 origin;
        glm::vec2 direction;

        bool RayVsCollider(RayHit& hit);
    };

    bool PointVsCollider(const glm::vec2& point);
}

class CollisionManager {
public:
    static std::map<int, Collision::Collider*> colliders;
};
