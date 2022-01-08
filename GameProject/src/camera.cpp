#include "camera.h"
#include "renderer.h"

#include "log.h"

const glm::vec3 worldUp = glm::vec3(0, 1, 0);

camera::camera()
	: position(0, 0, 0), rotation(0, -90, 0),
	  front(0,0,-1), right(0), up(worldUp)
{
	logger::info("INITIALIZED CAMERA ", this);
}

camera::~camera() { }

glm::mat4 camera::getViewMatrix() {
	front.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
	front.y = sin(glm::radians(rotation.y));
	front.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));

	front = glm::normalize(front);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

	return glm::lookAt(position, position + front, up);
}

void camera_manager::set_camera(std::string _id) {
	auto it = cameras.find(_id);
	if (it == cameras.end()) {
		cameras.emplace(_id, std::make_shared<camera>());

		it = cameras.find(_id);
		if (it != cameras.end()) {
			current_camera = it->second;
		}
	}
	else {
		current_camera = it->second;
	}
}

std::weak_ptr<camera> camera_manager::get_camera(std::string _id) {
	auto it = cameras.find(_id);
	if (it != cameras.end()) {
		return it->second;
	}

	return std::weak_ptr<camera>();
}

void camera_manager::update(double dt) {
	if (auto tmp_cam = current_camera.lock()) {
		renderer::view = tmp_cam->getViewMatrix();
	}
}