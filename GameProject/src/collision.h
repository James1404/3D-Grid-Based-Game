#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

namespace collision {
    struct box_collider {
        uint32_t id;

        glm::vec2 pos;
        glm::vec2 size;
    };

    box_collider* create_collider(glm::vec2 size);
    void delete_collider(box_collider* _collider);

    struct ray_data {
        glm::vec2 point;
        glm::vec2 normal;
        float dist;
        box_collider* col;
    };

    bool collider_vs_collider(box_collider* _collider);
    bool ray_vs_collider(ray_data& _hit, glm::vec2 _origin, glm::vec2 _direction);
    bool point_vs_collider(const glm::vec2& _point);
}
