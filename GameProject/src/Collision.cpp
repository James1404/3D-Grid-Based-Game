#include "Collision.h"

std::map<int, Collision::Rect*> CollisionManager::colliders;

bool Collision::RectVsRect(const Rect* rect1, const Rect* rect2) {
    return rect1->pos.x < rect2->pos.x + rect2->size.x &&
        rect1->pos.x + rect1->size.x > rect2->pos.x &&
        rect1->pos.y < rect2->pos.y + rect2->size.y &&
        rect1->pos.y + rect1->size.y > rect2->pos.y;
}

bool Collision::PointVsRect(const glm::vec2& point, const Rect* rect) {
    return (point.x >= rect->pos.x && point.y >= rect->pos.y && point.x < rect->pos.x + rect->size.x && point.y < rect->pos.y + rect->size.y);
}

bool Collision::RayVsRect(const Ray* ray, const Rect* target, RayHit& hit) {
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

void CollisionManager::AddCollider(int id, Collision::Rect* rect) {
    colliders[id] = rect;
}

void CollisionManager::UpdateCollider(int id, Collision::Rect* rect) {
    colliders[id] = rect;
}