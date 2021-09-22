#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

#include "entity.h"

struct collider {
    entity* owner;

    glm::vec2 pos;
    glm::vec2 size;
};

collider* create_collider(entity* _owner, glm::vec2 size);
void delete_collider(collider* _collider);

struct ray_data {
    glm::vec2 point;
    glm::vec2 normal;
    float dist;
    collider* col;
};

bool collider_vs_collider(collider* _collider);
bool ray_vs_collider(entity* _owner, ray_data& _hit, glm::vec2 _origin, glm::vec2 _direction);
bool point_vs_collider(const glm::vec2& _point);