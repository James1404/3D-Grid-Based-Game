#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <map>
#include <string>
#include <memory>

struct camera_t {
	glm::vec3 position;
	glm::vec3 rotation;

	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	
	glm::mat4 getViewMatrix();

	camera_t();
	~camera_t();
};

struct camera_manager_t {
	// TODO: change from shared to weak ptr
	std::map<std::string, std::shared_ptr<camera_t>> cameras;
	std::weak_ptr<camera_t> current_camera;

	void set_camera(std::string _id);
	std::weak_ptr<camera_t> get_camera(std::string _id);

	void update(double dt);
};
