#include "Collision.h"

std::map<int, Collision::Collider*> CollisionManager::colliders;

bool Collision::ColliderVsCollider(const Collider* collider1, const Collider* collider2) {
    return collider1->pos.x < collider2->pos.x + collider2->size.x &&
           collider1->pos.x + collider1->size.x > collider2->pos.x &&
           collider1->pos.y < collider2->pos.y + collider2->size.y &&
           collider1->pos.y + collider1->size.y > collider2->pos.y;
}

bool Collision::PointVsCollider(const glm::vec2& point, const Collider* collider) {
    return (point.x >= collider->pos.x && point.y >= collider->pos.y && point.x < collider->pos.x + collider->size.x && point.y < collider->pos.y + collider->size.y);
}

bool Collision::RayVsCollider(const Ray* ray, const Collider* target, RayHit& hit) {
    hit.normal = { 0,0 };
    hit.point = { 0,0 };

    glm::vec2 invdir = 1.0f / ray->direction;

    glm::vec2 t_near = (target->pos - ray->origin) * invdir;
    glm::vec2 t_far = (target->pos + target->size - ray->origin) * invdir;

    if (std::isnan(t_far.y) || std::isnan(t_far.x)) return false;
    if (std::isnan(t_near.y) || std::isnan(t_near.x)) return false;

    if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
    if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

    if (t_near.x > t_far.y || t_near.y > t_far.x) return false;

    hit.distance = std::max(t_near.x, t_near.y);

    float t_hit_far = std::min(t_far.x, t_far.y);

    if (t_hit_far < 0)
        return false;

    hit.point = ray->origin + hit.distance * ray->direction;

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

void CollisionManager::AddCollider(int id, Collision::Collider* collider) {
    colliders[id] = collider;
}