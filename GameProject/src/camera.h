#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <map>
#include <string>
#include <memory>

namespace camera {
	struct camera_interface {
		glm::mat4 view;
		virtual void update(double dt) {};
	};

	struct camera_data {
		std::map<std::string, std::shared_ptr<camera_interface>> cameras;
		std::shared_ptr<camera_interface> current_camera;
	};

	extern camera_data data;

	void register_camera(std::string id, std::shared_ptr<camera_interface> camera);
	void set_camera(std::string id);

	void update(double dt);
}