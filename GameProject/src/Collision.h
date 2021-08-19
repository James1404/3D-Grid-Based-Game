#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

// TODO: Add Layers for Collision
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

// TODO: Spatial Hashing
class CollisionManager {
public:
    static std::vector<Collider*> colliders;
};
