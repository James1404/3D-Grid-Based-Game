#include <glm.hpp>
#include <gtx/hash.hpp>
#include <queue>
#include <unordered_map>

#include "entity.h"

template<typename T, typename priority_t>
struct PriorityQueue {
	typedef std::pair<priority_t, T> PQElement;

	struct PQElementCompare {
		bool operator()(const PQElement& lhs, const PQElement& rhs) const {
			return lhs.first > rhs.first;
		}
	};

	std::priority_queue<PQElement, std::vector<PQElement>, PQElementCompare> elements;

	inline bool empty() const {
		return elements.empty();
	}

	inline void put(T item, priority_t priority) {
		elements.emplace(priority, item);
	}

	T get() {
		T best_item = elements.top().second;
		elements.pop();
		return best_item;
	}
};

inline int heuristic(glm::ivec2 a, glm::ivec2 b) {
	return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

void a_star_search(entity_manager& _graph, glm::ivec2 _start, glm::ivec2 _goal,
	std::unordered_map<glm::ivec2, glm::ivec2>& came_from,
	std::unordered_map<glm::ivec2, int>& cost_so_far) {
	PriorityQueue<glm::ivec2, int> frontier;
	frontier.put(_start, 0);

	came_from[_start] = _start;
	cost_so_far[_start] = 0;

	while (!frontier.empty()) {
		auto current = frontier.get();

		if (current == _goal)
			break;

		for (glm::ivec2 next : _graph.neighbors(current)) {
			int new_cost = cost_so_far[current] + 1;
			if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) {
				cost_so_far[next] = new_cost;
				int priority = new_cost + heuristic(next, _goal);
				frontier.put(next, priority);
				came_from[next] = current;
			}
		}
	}
}

std::vector<glm::ivec2> reconstruct_path(glm::ivec2 _start, glm::ivec2 _goal,
	std::unordered_map<glm::ivec2, glm::ivec2> came_from) {
	std::vector<glm::ivec2> path;
	glm::ivec2 current = _goal;
	while (current != _start) {
		path.push_back(current);
		current = came_from[current];
	}
	path.push_back(_start); // optional
	std::reverse(path.begin(), path.end());
	return path;
}