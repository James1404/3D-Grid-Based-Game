#include "camera.h"
#include "renderer.h"

camera::camera_data camera::data;

void camera::register_camera(std::string id, std::shared_ptr<camera_interface> camera) { data.cameras[id] = camera; }
void camera::set_camera(std::string id) { data.current_camera = data.cameras[id]; }
void camera::update(double dt) {
	if (data.current_camera != nullptr) {
		data.current_camera->update(dt);
		renderer::view = data.current_camera->view;
	}
}