#include "collision.h"

#include <memory>
#include <map>

#include "renderer.h"

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

void collision::draw_debug() {
    for (auto& col : collider_list) {
        renderer::debug::draw_box_wireframe(*col->pos, col->size, colour::pink);
    }
}

bool collision::box_vs_box(glm::vec2 pos1, glm::ivec2 size1, glm::vec2 pos2, glm::ivec2 size2) {
	if (pos1.x < pos2.x + size2.x &&
    	pos1.x + size1.x > pos2.x &&
        pos1.y < pos2.y + size2.y &&
		pos1.y + size1.y > pos2.y) {
		return true;
	}

    return false;
}

bool collision::point_vs_collider(const glm::vec2& _point, box_collider* _col) {
    if (_point.x >= _col->pos->x &&
        _point.y >= _col->pos->y &&
		_point.x <  _col->pos->x + _col->size.x &&
        _point.y <  _col->pos->y + _col->size.y) {
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
    if (line_vs_line(*_col->pos, { _col->pos->x, _col->pos->y + _col->size.y }, _origin, _direction + _origin, temp_point))
        { hit_points.insert({ glm::distance(_origin, temp_point), temp_point }); }
    if (line_vs_line(*_col->pos, { _col->pos->x + _col->size.x, _col->pos->y }, _origin, _direction + _origin, temp_point))
        { hit_points.insert({ glm::distance(_origin, temp_point), temp_point }); }
    if (line_vs_line(*_col->pos + (glm::vec2)_col->size, { _col->pos->x, _col->pos->y + _col->size.y }, _origin, _direction + _origin, temp_point))
        { hit_points.insert({ glm::distance(_origin, temp_point), temp_point }); }
    if (line_vs_line(*_col->pos + (glm::vec2)_col->size, { _col->pos->x + _col->pos->x, _col->pos->y }, _origin, _direction + _origin, temp_point))
        { hit_points.insert({ glm::distance(_origin, temp_point), temp_point }); }

    if (hit_points.empty()) return false;

    _hit.point = hit_points.begin()->second;
    _hit.dist = hit_points.begin()->first;
	_hit.col = _col;

    return true;
}

bool collision::check_box_collision(box_collider* _collider) {
    for (const auto& collider : collider_list) {
        if (_collider->id == collider->id)
            continue;

        if (box_vs_box(*_collider->pos, _collider->size, *collider->pos, collider->size)) {
            return true;
        }
	}
	
    return false;
}

bool collision::check_box_collision(glm::vec2 _pos, glm::ivec2 _size, uint32_t ignore_collision) {
    for (const auto& collider : collider_list) {
        if (collider->id == ignore_collision && ignore_collision != -100)
            continue;

        if (box_vs_box(_pos, _size, *collider->pos, collider->size)) {
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

bool collision::detect_and_resolve_collision(box_collider* _col, glm::vec2& _old_pos, glm::vec2& _pos, glm::vec2& _vel) {
    for (const auto& collider : collider_list) {
        if (_col->id == collider->id)
            continue;

        if (box_vs_box(*_col->pos, _col->size, *collider->pos, collider->size)) {
            glm::vec2 d = { 0,0 };

            glm::vec2 old_vel = _vel;

            if (_col->pos->x < collider->pos->x)
                d.x = collider->pos->x - (_col->pos->x + _col->size.x);
            else if (_col->pos->x > collider->pos->x)
                d.x = _col->pos->x - (collider->pos->x + collider->size.x);

            if (_col->pos->y < collider->pos->y)
                d.y = collider->pos->y - (_col->pos->y + _col->size.y);
            else if (_col->pos->y > collider->pos->y)
                d.y = _col->pos->y - (collider->pos->y + collider->size.y);

            glm::vec2 time_to_collide = { 0,0 };
            time_to_collide.x = _vel.x != 0 ? std::abs(d.x / _vel.x) : 0;
            time_to_collide.y = _vel.y != 0 ? std::abs(d.y / _vel.y) : 0;

            float shortestTime = 0;
            if (_vel.x != 0 && _vel.y == 0) {
                shortestTime = time_to_collide.x;
                _vel.x *= -shortestTime;
            }
            else if (_vel.x == 0 && _vel.y != 0) {
                shortestTime = time_to_collide.y;
                _vel.y *= -shortestTime;
            }
            else {
                shortestTime = std::min(std::abs(time_to_collide.x), std::abs(time_to_collide.y));
                _vel *= -shortestTime;
            }
            
            if (shortestTime == time_to_collide.x) {
                if (!box_vs_box({_col->pos->x, _col->pos->y + old_vel.y}, _col->size, *collider->pos, collider->size)) {
                    _vel.y = old_vel.y;
                }
            }

            if (shortestTime == time_to_collide.y) {
                if (!box_vs_box({ _col->pos->x + old_vel.x, _col->pos->y }, _col->size, *collider->pos, collider->size)) {
                    _vel.x = old_vel.x;
                }
            }

            _pos = _old_pos + _vel;

            return true;
        }
    }

    return false;
}