#pragma once
#include <string>
#include <glm.hpp>

namespace input {
	void init();
	void clean();

	void update();

	bool button_down(std::string button);
	bool button_pressed(std::string button);
	bool button_released(std::string button);

	const glm::ivec2* get_mouse_pos();

	void save();
	void load();
}