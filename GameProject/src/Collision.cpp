#include "Collision.h"

std::map<int, Collision::Collider*> CollisionManager::colliders;

unsigned int Collision::Collider::currentID = 0;

Collision::Collider::Collider() : id(currentID++) {
    CollisionManager::colliders[id] = this;
}

Collision::Collider::~Collider() { }

bool Collision::Collider::ColliderVsCollider() {
    for (const auto& collider : CollisionManager::colliders) {
        if (collider.first != this->id) {
            if (this->pos.x < collider.second->pos.x + collider.second->size.x &&
                this->pos.x + this->size.x >            collider.second->pos.x &&
                this->pos.y < collider.second->pos.y + collider.second->size.y &&
                this->pos.y + this->size.y >            collider.second->pos.y) {
                return true;
            }
        }
    }

    return false;
}

bool Collision::Ray::RayVsCollider(RayHit& hit) {
    for (const auto& collider : CollisionManager::colliders) {
        hit.normal = { 0,0 };
        hit.point = { 0,0 };

        glm::vec2 invdir = 1.0f / this->direction;

        glm::vec2 t_near = (collider.second->pos - this->origin) * invdir;
        glm::vec2 t_far = (collider.second->pos + collider.second->size - this->origin) * invdir;

        if (std::isnan(t_far.y) || std::isnan(t_far.x)) continue;
        if (std::isnan(t_near.y) || std::isnan(t_near.x)) continue;

        if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
        if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

        if (t_near.x > t_far.y || t_near.y > t_far.x) continue;

        hit.distance = std::max(t_near.x, t_near.y);

        float t_hit_far = std::min(t_far.x, t_far.y);

        if (t_hit_far < 0)
            continue;

        hit.point = this->origin + hit.distance * this->direction;

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

        return true;
    }

    return false;
}

bool Collision::PointVsCollider(const glm::vec2& point) {
    for (const auto& collider : CollisionManager::colliders) {
        if (point.x >= collider.second->pos.x &&
            point.y >= collider.second->pos.y &&
            point.x < collider.second->pos.x + collider.second->size.x &&
            point.y < collider.second->pos.y + collider.second->size.y) {
            return true;
        }
    }

    return false;
}