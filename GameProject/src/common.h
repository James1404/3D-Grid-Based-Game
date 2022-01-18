#pragma once
#include <glm.hpp>
#include <queue>

namespace common {
	inline float lerp(const float a, const float b, const float t) {
		return (a * (1.0f - t) + (b * t));
	}

	inline glm::vec2 lerp(const glm::vec2 a, const glm::vec2 b, const float t) {
		return glm::vec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
	}

	inline glm::vec3 lerp(const glm::vec3 a, const glm::vec3 b, const float t) {
		return glm::vec3(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t));
	}

	inline glm::vec2 move_towards(const glm::vec2 pos, const glm::vec2 target, const float step) {
		const glm::vec2 delta = target - pos;
		const float len2 = glm::dot(delta, delta);

		if (len2 < step * step)
			return target;

		const glm::vec2 direction = delta / glm::sqrt(len2);

		return pos + step * direction;
	}

	inline glm::vec3 move_towards(const glm::vec3 pos, const glm::vec3 target, const float step) {
		const glm::vec3 delta = target - pos;
		const float len2 = glm::dot(delta, delta);

		if (len2 < step * step)
			return target;

		const glm::vec3 direction = delta / glm::sqrt(len2);

		return pos + step * direction;
	}

	inline glm::vec2 vec_floor(glm::vec2 vec) {
		return glm::vec2(floorf(vec.x), floorf(vec.y));
	}

	inline glm::vec3 vec_floor(glm::vec3 vec) {
		return glm::vec3(floorf(vec.x), floorf(vec.y), floorf(vec.z));
	}

	inline glm::ivec2 vec_to_ivec(glm::vec2 vec) {
		return (glm::ivec2)vec_floor(vec);
	}

	inline glm::ivec3 vec_to_ivec(glm::vec3 vec) {
		return (glm::ivec3)vec_floor(vec);
	}

	inline glm::vec3 sqrt_magnitude(glm::vec3 vec)
	{
		float length = sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
		if (length != 0) {
			vec /= length;
		}

		return vec;
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
}