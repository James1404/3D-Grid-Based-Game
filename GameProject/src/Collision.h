#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

#include "Entity.h"

struct collider {
    collider();
    ~collider();

    void init_collider(entity* _owner);

    entity* owner;

    glm::vec2 pos;
    glm::vec2 size;

    bool collider_vs_collider();
};

struct ray_hit {
    glm::vec2 point;
    glm::vec2 normal;
    float dist;
    collider* col;
};

bool ray_vs_collider(entity* owner, ray_hit& hit, glm::vec2 origin, glm::vec2 direction);
bool point_vs_collider(const glm::vec2& point);