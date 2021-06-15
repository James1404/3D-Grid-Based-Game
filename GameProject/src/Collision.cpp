#include "Collision.h"

std::map<std::string, Collider> CollisionManager::colliders;

void CollisionManager::AddCollider(std::string name, glm::vec2 pos, glm::vec2 size) {
    Collider col;
    col.pos = pos;
    col.size = size;

    colliders[name] = col;
}

void CollisionManager::UpdateCollider(std::string name, glm::vec2 pos, glm::vec2 size) {
    colliders[name].pos = pos;
    colliders[name].size = size;
}