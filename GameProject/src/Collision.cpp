#include "Collision.h"

std::vector<Collider*> CollisionManager::colliders;

Collider::Collider()
    : pos(0,0), size(0,0) {
    CollisionManager::colliders.push_back(this);
}

Collider::~Collider() { }

void Collider::InitCollider(Entity* _owner) {
    this->owner = _owner;
}

bool Collider::ColliderVsCollider() {
    for (const auto& collider : CollisionManager::colliders) {
        if (collider->owner->id != this->owner->id) {
            if (this->pos.x < collider->pos.x + collider->size.x &&
                this->pos.x + this->size.x >    collider->pos.x &&
                this->pos.y < collider->pos.y + collider->size.y &&
                this->pos.y + this->size.y >    collider->pos.y) {
                return true;
            }
        }
    }

    return false;
}

// TODO: Fix Raycasting Implementation
bool RayVsCollider(Entity* owner, RayHit& hit, glm::vec2 origin, glm::vec2 direction) {
    for (const auto& collider : CollisionManager::colliders) {
        if (collider->owner->id != owner->id) {
            hit.normal = { 0,0 };
            hit.point = { 0,0 };

            glm::vec2 invdir = 1.0f / direction;

            glm::vec2 t_near = (collider->pos - origin) * invdir;
            glm::vec2 t_far = (collider->pos + collider->size - origin) * invdir;

            if (std::isnan(t_far.y) || std::isnan(t_far.x)) continue;
            if (std::isnan(t_near.y) || std::isnan(t_near.x)) continue;

            if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
            if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

            if (t_near.x > t_far.y || t_near.y > t_far.x) continue;

            hit.distance = std::max(t_near.x, t_near.y);

            float t_hit_far = std::min(t_far.x, t_far.y);

            if (t_hit_far < 0)
                continue;

            hit.point = origin + hit.distance * direction;

            if (t_near.x > t_near.y)
                if (invdir.x < 0)
                    hit.normal = { 1, 0 };
                else
                    hit.normal = { -1, 0 };
            else if (t_near.x < t_near.y)
                if (invdir.y < 0)
                    hit.normal = { 0, 1 };
                else
                    hit.normal = { 0, -1 };

            hit.collider = collider;

            return true;
        }
    }

    return false;
}

bool PointVsCollider(const glm::vec2& point) {
    for (const auto& collider : CollisionManager::colliders) {
        if (point.x >= collider->pos.x &&
            point.y >= collider->pos.y &&
            point.x <  collider->pos.x + collider->size.x &&
            point.y <  collider->pos.y + collider->size.y) {
            return true;
        }
    }

    return false;
}