#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

#include "Entity.h"

struct Collider {
    Collider();
    ~Collider();

    void InitCollider(Entity* _owner);

    Entity* owner;

    glm::vec2 pos;
    glm::vec2 size;

    bool ColliderVsCollider();
};

struct RayHit {
    glm::vec2 point;
    glm::vec2 normal;
    float distance;
    Collider* collider;
};

bool RayVsCollider(Entity* owner, RayHit& hit, glm::vec2 origin, glm::vec2 direction);
bool PointVsCollider(const glm::vec2& point);

// TODO: Spatial Hashing
class CollisionManager {
public:
    static std::vector<Collider*> colliders;
};
