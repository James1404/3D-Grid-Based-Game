#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <map>
#include <string>
#include <memory>
#include <cassert>

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

struct camera_manager_t
{
	std::map<std::string, std::shared_ptr<camera_t>> cameras;
	std::weak_ptr<camera_t> current_camera;

	std::weak_ptr<camera_t> get_camera(std::string _id);
	std::weak_ptr<camera_t> get_current_camera();

	void update();
	void set_camera(std::string _id);

	void shutdown();

	static camera_manager_t& get()
	{
		static camera_manager_t* instance = NULL;
		if(instance == NULL)
		{
			instance = new camera_manager_t;
		}
		assert(instance);
		return *instance;
	}
};
