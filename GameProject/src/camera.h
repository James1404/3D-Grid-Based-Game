#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <map>
#include <string>
#include <memory>

struct camera{
	glm::vec3 position;
	glm::vec3 rotation;

	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	
	glm::mat4 getViewMatrix();

	camera();
	~camera();
};

struct camera_manager {
	// TODO: change from shared to weak ptr
	std::map<std::string, std::shared_ptr<camera>> cameras;
	std::weak_ptr<camera> current_camera;

	void set_camera(std::string _id);
	std::weak_ptr<camera> get_camera(std::string _id);

	void update(double dt);
};