#pragma once
#include <glm.hpp>
#include <vector>
#include <map>
#include <string>

namespace collision {
    struct box_collider {
        uint32_t id;

        glm::vec2 pos;
        glm::ivec2 size;
    };

    box_collider* create_collider();
    void delete_collider(box_collider* _collider);

    struct ray_data {
        glm::vec2 point;
        // glm::vec2 normal;
        float dist;
        box_collider* col;
    };

	bool box_vs_box(glm::vec2 pos1, glm::ivec2 size1, glm::vec2 pos2, glm::ivec2 size2);
    bool point_vs_collider(const glm::vec2& _point, box_collider* _col);
    bool line_vs_line(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, glm::vec2& intersection);
    bool line_vs_collider(ray_data& _hit, glm::vec2 _origin, glm::vec2 _direction, box_collider* _col);

    bool check_box_collision(box_collider* _collider);
    bool check_box_collision(glm::vec2 _pos, glm::ivec2 _size);

    bool check_linecast_collision(ray_data& _hit, glm::vec2 _origin, glm::vec2 _direction);
    bool check_point_collision(const glm::vec2& _point);
}
