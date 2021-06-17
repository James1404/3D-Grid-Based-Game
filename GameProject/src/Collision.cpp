#include "Collision.h"

std::map<int, Collider> CollisionManager::colliders;

void CollisionManager::AddCollider(int id, glm::vec2 pos, glm::vec2 size) {
    Collider col;
    col.pos = pos;
    col.size = size;

    colliders[id] = col;
}

void CollisionManager::UpdateCollider(int id, glm::vec2 pos, glm::vec2 size) {
    colliders[id].pos = pos;
    colliders[id].size = size;
}

bool Collider::isCollision(const Collider& col)
{
    return this->pos.x < col.pos.x + col.size.x &&
        this->pos.x + this->size.x > col.pos.x &&
        this->pos.y < col.pos.y + col.size.y &&
        this->pos.y + this->size.y > col.pos.y;
}

glm::vec2 Collider::CalculateDistance(const Collider& col)
{
    glm::vec2 d = { 0,0 };

    if (this->pos.x < col.pos.x) {
        d.x = col.pos.x - (this->pos.x + this->size.x);
    }
    else if (this->pos.x > col.pos.x) {
        d.x = this->pos.x - (col.pos.x + col.size.x);
    }

    if (this->pos.y < col.pos.y) {
        d.y = col.pos.y - (this->pos.y + this->size.y);
    }
    else if (this->pos.y > col.pos.y) {
        d.y = this->pos.y - (col.pos.y + col.size.y);
    }

    return d;
}
