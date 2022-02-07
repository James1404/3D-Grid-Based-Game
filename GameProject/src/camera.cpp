#include "camera.h"
#include "renderer.h"

#include "log.h"

const glm::vec3 worldUp = glm::vec3(0, 1, 0);

static std::map<std::string, std::shared_ptr<camera_t>> cameras;
static std::weak_ptr<camera_t> current_camera;

camera_t::camera_t()
	: position(0, 0, 0), rotation(0, -90, 0),
	  front(0,0,-1), right(0), up(worldUp)
{
	log_info("INITIALIZED CAMERA ", this);
}

camera_t::~camera_t() { }

glm::mat4 camera_t::getViewMatrix()
{
	// pitch = rotation.x | yaw = rotation.y
	front.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
	front.y = sin(glm::radians(rotation.x));
	front.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));

	front = glm::normalize(front);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

	return glm::lookAt(position, position + front, up);
}

void update_camera()
{
	if (auto tmp_cam = current_camera.lock())
	{
		view_matrix = tmp_cam->getViewMatrix();
	}
}

void set_camera(std::string _id)
{
	auto it = cameras.find(_id);
	if (it == cameras.end())
	{
		cameras.emplace(_id, std::make_shared<camera_t>());

		it = cameras.find(_id);
		if (it != cameras.end())
		{
			current_camera = it->second;
		}
	}
	else
	{
		if (current_camera.lock() != it->second)
		{
			current_camera = it->second;
		}
	}
}

std::weak_ptr<camera_t> get_camera(std::string _id)
{
	auto it = cameras.find(_id);
	if (it != cameras.end())
	{
		return it->second;
	}

	return std::weak_ptr<camera_t>();
}

std::weak_ptr<camera_t> get_current_camera()
{
	if (current_camera.lock())
	{
		return current_camera;
	}
	return std::weak_ptr<camera_t>();
}
