#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

namespace Collision {
    struct Rect {
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

    bool RectVsRect(const Rect* rect1, const Rect* rect2);
    bool PointVsRect(const glm::vec2& point, const Rect* rect);
    bool RayVsRect(const Ray* ray, const Rect* target, RayHit& hit);
}

class CollisionManager {
public:
    static std::map<int, Collision::Rect*> colliders;

    static void AddCollider(int id, Collision::Rect* rect);
    static void UpdateCollider(int id, Collision::Rect* rect);
};
