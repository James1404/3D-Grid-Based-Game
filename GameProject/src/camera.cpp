#include "camera.h"
#include "renderer.h"

#include "log.h"

const glm::vec3 worldUp = glm::vec3(0, 1, 0);

camera_t::camera_t()
	: position(0, 0, 0), rotation(0, -90, 0),
	  front(0,0,-1), right(0), up(worldUp)
{
	log_info("INITIALIZED CAMERA ", this);
}

camera_t::~camera_t() { }

glm::mat4 camera_t::getViewMatrix() {
	// pitch = rotation.x | yaw = rotation.y
	front.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
	front.y = sin(glm::radians(rotation.x));
	front.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));

	front = glm::normalize(front);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

	return glm::lookAt(position, position + front, up);
}

void camera_manager_t::set_camera(std::string _id) {
	auto it = cameras.find(_id);
	if (it == cameras.end()) {
		cameras.emplace(_id, std::make_shared<camera_t>());

		it = cameras.find(_id);
		if (it != cameras.end()) {
			current_camera = it->second;
		}
	}
	else {
		if (current_camera.lock() != it->second) {
			current_camera = it->second;
		}
	}
}

std::weak_ptr<camera_t> camera_manager_t::get_camera(std::string _id) {
	auto it = cameras.find(_id);
	if (it != cameras.end()) {
		return it->second;
	}

	return std::weak_ptr<camera_t>();
}

void camera_manager_t::update(double dt) {
	if (auto tmp_cam = current_camera.lock()) {
		renderer::view = tmp_cam->getViewMatrix();
	}
}
