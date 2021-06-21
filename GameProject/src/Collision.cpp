#include "Collision.h"

std::map<int, Rect*> CollisionManager::colliders;

bool Collision::RectVsRect(const Rect* rect1, const Rect* rect2) {
    return rect1->pos.x < rect2->pos.x + rect2->size.x &&
        rect1->pos.x + rect1->size.x > rect2->pos.x &&
        rect1->pos.y < rect2->pos.y + rect2->size.y &&
        rect1->pos.y + rect1->size.y > rect2->pos.y;
}

bool Collision::RayVsRect(const Ray* ray, const Rect* rect) {
    return false;
}

void CollisionManager::AddCollider(int id, Rect* rect) {
    colliders[id] = rect;
}

void CollisionManager::UpdateCollider(int id, Rect* rect) {
    colliders[id] = rect;
}