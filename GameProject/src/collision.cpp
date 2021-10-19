#include "collision.h"

#include <memory>

// TODO: Spatial Hashing
std::vector<std::unique_ptr<collision::box_collider>> collider_list;

static uint32_t current_id = 0;
collision::box_collider* collision::create_collider(glm::vec2 _size) {
    auto c = std::make_unique<box_collider>();
    c->id = current_id++;
    c->size = _size;

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
        if (_col1->pos.x < _col2->pos.x +   _col2->size.x &&
            _col1->pos.x + _col1->size.x >  _col2->pos.x &&
            _col1->pos.y < _col2->pos.y +   _col2->size.y&&
			_col1->pos.y + _col1->size.y >  _col2->pos.y) {
			return true;
        }
	}

    return false;
}

// TODO: Fix Raycasting Implementation
bool collision::ray_vs_collider(ray_data& _hit, glm::vec2 _origin, glm::vec2 _direction, box_collider* _col) {
    _hit.normal = { 0,0 };
    _hit.point = { 0,0 };

    glm::vec2 invdir = 1.0f / _direction;

    glm::vec2 t_near = (_col->pos - _origin) * invdir;
    glm::vec2 t_far = (_col->pos + _col->size - _origin) * invdir;

    if (std::isnan(t_far.y) || std::isnan(t_far.x)) return false;
    if (std::isnan(t_near.y) || std::isnan(t_near.x)) return false;

    if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
    if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

    if (t_near.x > t_far.y || t_near.y > t_far.x) return false;

    _hit.dist = std::max(t_near.x, t_near.y);

    float t_hit_far = std::min(t_far.x, t_far.y);

    if (t_hit_far < 0)
    	return false;

    _hit.point = _origin + _hit.dist * _direction;

    if (t_near.x > t_near.y)
        if (invdir.x < 0)
            _hit.normal = { 1, 0 };
        else
            _hit.normal = { -1, 0 };
    else if (t_near.x < t_near.y)
        if (invdir.y < 0)
            _hit.normal = { 0, 1 };
        else
            _hit.normal = { 0, -1 };

    _hit.col = _col;

    return true;
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

bool collision::check_box_collision(box_collider* _collider) {
    for (const auto& collider : collider_list) {
        if (collider_vs_collider(_collider, collider.get())) {
            return true;
        }

        /*
        if (collider->id != _collider->id) {
            if (_collider->pos.x < collider->pos.x + collider->size.x &&
                _collider->pos.x + _collider->size.x >    collider->pos.x &&
                _collider->pos.y < collider->pos.y + collider->size.y &&
                _collider->pos.y + _collider->size.y >    collider->pos.y) {
                return true;
            }
        }
        */
    }

    return false;
}

bool collision::check_ray_collision(ray_data& _hit, glm::vec2 _origin, glm::vec2 _direction) {
    for (auto& collider : collider_list) {
        if (ray_vs_collider(_hit, _origin, _direction, collider.get())) {
            return true;
        }

        /*
        _hit.normal = { 0,0 };
        _hit.point = { 0,0 };

        glm::vec2 invdir = 1.0f / _direction;

        glm::vec2 t_near = (collider->pos - _origin) * invdir;
        glm::vec2 t_far = (collider->pos + collider->size - _origin) * invdir;

        if (std::isnan(t_far.y) || std::isnan(t_far.x)) continue;
        if (std::isnan(t_near.y) || std::isnan(t_near.x)) continue;

        if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
        if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);

        if (t_near.x > t_far.y || t_near.y > t_far.x) continue;

        _hit.dist = std::max(t_near.x, t_near.y);

        float t_hit_far = std::min(t_far.x, t_far.y);

        if (t_hit_far < 0)
            continue;

        _hit.point = _origin + _hit.dist * _direction;

        if (t_near.x > t_near.y)
            if (invdir.x < 0)
                _hit.normal = { 1, 0 };
            else
                _hit.normal = { -1, 0 };
        else if (t_near.x < t_near.y)
            if (invdir.y < 0)
                _hit.normal = { 0, 1 };
            else
                _hit.normal = { 0, -1 };

        _hit.col = collider.get();

        return true;
        */
    }

    return false;
}

bool collision::check_point_collision(const glm::vec2& _point) {
    for (const auto& collider : collider_list) {
        if (point_vs_collider(_point, collider.get())) {
            return true;
        }

        /*
        if (_point.x >= collider->pos.x &&
            _point.y >= collider->pos.y &&
            _point.x <  collider->pos.x + collider->size.x &&
            _point.y <  collider->pos.y + collider->size.y) {
            return true;
        }
        */
    }

    return false;
}