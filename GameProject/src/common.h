#pragma once
#include <glm.hpp>
#include <queue>
#include <tuple>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/matrix_decompose.hpp>
#include <glm.hpp>

inline float lerp(const float a, const float b, const float t)
{
	return (a * (1.0f - t) + (b * t));
}

inline glm::vec2 lerp(const glm::vec2 a, const glm::vec2 b, const float t)
{
	return glm::vec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

inline glm::vec3 lerp(const glm::vec3 a, const glm::vec3 b, const float t)
{
	return glm::vec3(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t));
}

inline glm::vec2 move_towards(const glm::vec2 pos, const glm::vec2 target, const float step)
{
	const glm::vec2 delta = target - pos;
	const float len2 = glm::dot(delta, delta);

	if (len2 < step * step)
	{
		return target;
	}

	const glm::vec2 direction = delta / glm::sqrt(len2);

	return pos + step * direction;
}

inline glm::vec3 move_towards(const glm::vec3 pos, const glm::vec3 target, const float step)
{
	const glm::vec3 delta = target - pos;
	const float len2 = glm::dot(delta, delta);

	if (len2 < step * step)
	{
		return target;
	}

	const glm::vec3 direction = delta / glm::sqrt(len2);

	return pos + step * direction;
}

inline glm::vec2 vec_floor(glm::vec2 vec)
{
	return glm::vec2(floorf(vec.x), floorf(vec.y));
}

inline glm::vec3 vec_floor(glm::vec3 vec)
{
	return glm::vec3(floorf(vec.x), floorf(vec.y), floorf(vec.z));
}

inline glm::ivec2 vec_to_ivec(glm::vec2 vec)
{
	return (glm::ivec2)vec_floor(vec);
}

inline glm::ivec3 vec_to_ivec(glm::vec3 vec)
{
	return (glm::ivec3)vec_floor(vec);
}

inline glm::vec3 sqrt_magnitude(glm::vec3 vec)
{
	float length = sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
	if (length != 0)
	{
		vec /= length;
	}

	return vec;
}

inline glm::vec3 shortest_vector_value(glm::vec3 vec)
{
	glm::vec3 abs_vec = glm::abs(vec);

	if (abs_vec.x > abs_vec.y && abs_vec.x > abs_vec.z)
	{
		vec.y = 0;
		vec.z = 0;
	}
	else if (abs_vec.y > abs_vec.x && abs_vec.y > abs_vec.z)
	{
		vec.x = 0;
		vec.z = 0;
	}
	else if (abs_vec.z > abs_vec.y && abs_vec.z > abs_vec.x)
	{
		vec.x = 0;
		vec.y = 0;
	}

	return vec;
}

inline std::tuple<glm::vec3,glm::vec3> swap_for_smallest_vector(glm::vec3 vec1, glm::vec3 vec2)
{
	if(vec1.x < vec2.x)
	{
		float start = vec1.x;
		vec1.x = vec2.x;
		vec2.x = start;
	}

	if(vec1.y < vec2.y)
	{
		float start = vec1.y;
		vec1.y = vec2.y;
		vec2.y = start;
	}

	if(vec1.z < vec2.z)
	{
		float start = vec1.z;
		vec1.z = vec2.z;
		vec2.z = start;
	}

	return std::make_tuple(vec1, vec2);
}

inline glm::vec3 round_vec_up_to_nearest(glm::vec3 vec)
{
	vec.x = (vec.x > 0) ? std::ceil(vec.x) : std::floor(vec.x);
	vec.y = (vec.y > 0) ? std::ceil(vec.y) : std::floor(vec.y);
	vec.z = (vec.z > 0) ? std::ceil(vec.z) : std::floor(vec.z);

	return vec;
}

inline bool decompose_transform(const glm::mat4& transform, glm::vec3& out_translation, glm::vec3& out_rotation, glm::vec3& out_scale)
{
	using namespace glm;
	using T = float;

	mat4 LocalMatrix(transform);

	if(epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
		return false;

	if(epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
		epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
		epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
	{
		LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
		LocalMatrix[3][3] = static_cast<T>(1);
	}

	out_translation = vec3(LocalMatrix[3]);
	LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

	vec3 Row[3], Pdum3;

	for (length_t i = 0; i < 3; ++i)
		for (length_t j = 0; j < 3; ++j)
			Row[i][j] = LocalMatrix[i][j];

	out_scale.x = length(Row[0]);
	Row[0] = detail::scale(Row[0], static_cast<T>(1));
	out_scale.y = length(Row[1]);
	Row[1] = detail::scale(Row[1], static_cast<T>(1));
	out_scale.z = length(Row[2]);
	Row[2] = detail::scale(Row[2], static_cast<T>(1));

#if 0
	Pdum3 = cross(Row[1], Row[2]);
	if (dot(Row[0], Pdum3) < 0)
	{
		for (length_t i = 0; i < 3; i++)
		{
			scale[i] *= static_cast<T>(-1);
			Row[i] *= static_cast<T>(-1);
		}
	}
#endif

	out_rotation.y = asin(-Row[0][2]);
	if(cos(out_rotation.y) != 0)
	{
		out_rotation.x = atan2(Row[1][2], Row[2][2]);
		out_rotation.z = atan2(Row[0][1], Row[0][0]);
	}
	else
	{
		out_rotation.x = atan2(-Row[2][0], Row[1][1]);
		out_rotation.z = 0;
	}

	return true;
}

template<class _Ty, int _TSize>
class Limited_Queue {
public:
	_Ty get() {
		if (queue.empty())
			return _Ty();

		_Ty _item = queue.front();
		queue.pop();
		return _item;
	}

	void push(_Ty type) {
		if (queue.size() >= _TSize)
			return;

		queue.push(type);
	}

	void push_at_front(_Ty type) {
		if (!queue.empty() || queue.size() >= _TSize)
			return;

		queue.push(type);
	}

	void clear() {
		std::queue<_Ty> empty;
		std::swap(queue, empty);
	}

	bool front_equals(_Ty type) const {
		return !queue.empty() && queue.front() == type;
	}
private:
	std::queue<_Ty> queue;
};

inline bool is_moving(transform_t& visual_transform, glm::ivec3& grid_pos)
{
	return (visual_transform.position != (glm::vec3)grid_pos);
}

inline void set_grid_pos(glm::ivec3& grid_pos, glm::ivec3& previous_grid_pos, glm::vec3 new_grid_pos)
{
	previous_grid_pos = grid_pos;
	grid_pos = new_grid_pos;
}

inline void revert_grid_pos(glm::ivec3& grid_pos, glm::ivec3& previous_grid_pos)
{
	grid_pos = previous_grid_pos;
}

inline void interp_visuals(transform_t& visual_transform, glm::ivec3& grid_pos, double dt, float interp_speed)
{
	// TODO: implement lerping instead of move_towards
	//visual_transform.position = lerp(visual_transform.position, (glm::vec3)grid_pos, interp_speed * dt);
	visual_transform.position = move_towards(visual_transform.position, grid_pos, interp_speed * dt);
}
