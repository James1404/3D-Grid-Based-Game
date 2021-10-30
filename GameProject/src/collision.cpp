#include "collision.h"

#include <memory>
#include <map>

// TODO: Spatial Hashing
std::vector<std::unique_ptr<collision::box_collider>> collider_list;

static uint32_t current_id = 0;
collision::box_collider* collision::create_collider() {
    auto c = std::make_unique<box_collider>();
    c->id = current_id++;

    auto pointer = c.get();

    collider_list.push_back(std::move(c));

    printf(" - COLLIDER CREATED\n");

    return pointer;
}

void collision::delete_collider(box_collider* _collider) {
    if (!collider_list.empty()) {
        for (auto it = collider_list.begin(); it != collider_list.end();) {
            if (it->get() == _collider) {
                printf(" - DELETED COLLIDER LOC: %p\n", it->get());
                it = collider_list.erase(it);
            }
            else {
                ++it;
            }
        }
    }
}

bool collision::collider_vs_collider(box_collider* _col1, box_collider* _col2) {
	if (_col1->id != _col2->id) {
        if (_col1->pos.x < _col2->pos.x +  _col2->size.x &&
            _col1->pos.x + _col1->size.x > _col2->pos.x &&
            _col1->pos.y < _col2->pos.y +  _col2->size.y&&
			_col1->pos.y + _col1->size.y > _col2->pos.y) {
			return true;
        }
	}

    return false;
}

bool collision::point_vs_collider(const glm::vec2& _point, box_collider* _col) {
    if (_point.x >= _col->pos.x &&
        _point.y >= _col->pos.y &&
		_point.x <  _col->pos.x + _col->size.x &&
        _point.y <  _col->pos.y + _col->size.y) {
        return true;
    }

    return false;
}

bool collision::line_vs_line(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, glm::vec2& intersection) {
    float ax = b.x - a.x;
    float ay = b.y - a.y;

    float bx = c.x - d.x;
    float by = c.y - d.y;

    float dx = c.x - a.x;
    float dy = c.y - a.y;

    const float den = ax * by - ay * bx;

    if (den == 0) return false;

    const float t = (dx * by - dy * bx) / den;
    const float u = (ax * dy - ay * dx) / den;

    if (!(t < 0 || t > 1 || u < 0 || u > 1)) {
        intersection.x = a.x + t * ax;
        intersection.y = a.y + t * ay;
        return true;
    }

    return false;
}

// TODO: Fix Raycasting Implementation
bool collision::line_vs_collider(ray_data& _hit, glm::vec2 _origin, glm::vec2 _direction, box_collider* _col) {
    std::map<float, glm::vec2> hit_points;

	glm::vec2 temp_point;
    if (line_vs_line(_col->pos, { _col->pos.x, _col->pos.y + _col->size.y }, _origin, _direction, temp_point))
        { hit_points.insert({ glm::distance(_origin, temp_point), temp_point }); }
    if (line_vs_line(_col->pos, { _col->pos.x + _col->size.x, _col->pos.y }, _origin, _direction, temp_point))
        { hit_points.insert({ glm::distance(_origin, temp_point), temp_point }); }
    if (line_vs_line(_col->pos + (glm::vec2)_col->size, { _col->pos.x, _col->pos.y + _col->size.y }, _origin, _direction, temp_point))
        { hit_points.insert({ glm::distance(_origin, temp_point), temp_point }); }
    if (line_vs_line(_col->pos + (glm::vec2)_col->size, { _col->pos.x + _col->pos.x, _col->pos.y }, _origin, _direction, temp_point))
        { hit_points.insert({ glm::distance(_origin, temp_point), temp_point }); }

    if (hit_points.empty()) return false;

    _hit.point = hit_points.begin()->second;
    _hit.dist = hit_points.begin()->first;
	_hit.col = _col;

    return true;
}

bool collision::check_box_collision(box_collider* _collider) {
    for (const auto& collider : collider_list) {
        if (collider_vs_collider(_collider, collider.get())) {
            return true;
        }
	}
	
    return false;
}

bool collision::check_linecast_collision(ray_data& _hit, glm::vec2 _origin, glm::vec2 _direction) {
    std::map<float, ray_data> hits;
    
    for (const auto& collider : collider_list) {
        ray_data temp_hit;
        if (line_vs_collider(temp_hit, _origin, _direction, collider.get())) {
            hits.insert({ glm::distance(_origin, temp_hit.point), temp_hit });
        }
    }

    if (hits.empty()) return false;

    _hit = hits.begin()->second;
    return true;
}

bool collision::check_point_collision(const glm::vec2& _point) {
    for (const auto& collider : collider_list) {
        if (point_vs_collider(_point, collider.get())) {
            return true;
        }
    }

    return false;
}
