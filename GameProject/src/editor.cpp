#include "editor.h"

#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <string>

#include "input.h"
#include "renderer.h"
#include "entity.h"
#include "camera.h"
#include "log.h"

#include "player.h"
#include "world_entities.h"

void editor_manager::init(entity_manager& _manager) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(renderer::window, renderer::context);
	ImGui_ImplOpenGL3_Init("#version 330");

	manager = &_manager;

	logger::info("INITIALIZED EDITOR");
}

void editor_manager::clean() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	logger::info("-----------------\n");
	logger::info("EDITOR CLEANED UP\n");
}

void editor_manager::move_cursor(glm::ivec3 _vel) {
	if (_vel == glm::ivec3(0))
		return;

	cursor_pos += _vel;

	if (is_grabbed) {
		for (auto selected : selected_entities) {
			if (auto tmp_entity = selected.lock()) {
				tmp_entity->grid_pos += _vel;
				tmp_entity->visual_pos = tmp_entity->grid_pos;
			}
		}
	}
}

void editor_manager::placement_cam_mode_update(double dt, std::shared_ptr<camera> cam) {
	cursor_pos = common::vec_to_ivec(cursor_pos);
	glm::ivec3 vel{ 0 };
	if (!(input::button_down("MoveUp") && input::button_down("MoveDown"))) {
		if (input::button_down("MoveUp")) {
			vel = { 0,0, -1 };
		}
		else if (input::button_down("MoveDown")) {
			vel = { 0,0, 1 };
		}
	}

	if (!(input::button_down("MoveLeft") && input::button_down("MoveRight"))) {
		if (input::button_down("MoveLeft")) {
			vel = { -1,0,0 };
		}
		else if (input::button_down("MoveRight")) {
			vel = { 1,0,0 };
		}
	}

	if (input::button_down("IncreaseHeight")) {
		vel = { 0,1,0 };
	}
	else if (input::button_down("DecreaseHeight")) {
		vel = { 0,-1,0 };
	}

	glm::vec3 offset = glm::vec3(0, 6, 5);
	cam->position = ((glm::vec3)cursor_pos + offset);

	cam->rotation.x = -50;
	cam->rotation.y = -90;

	move_cursor(vel);

	renderer::debug::draw_box_wireframe(cursor_pos, glm::vec3(1), colour::white);


	for (auto selected : selected_entities) {
		if (auto tmp_entity = selected.lock()) {
			renderer::debug::draw_box_wireframe(tmp_entity->grid_pos, glm::vec3(1), colour::white);
		}
	}

	if (input::key_down(SDL_SCANCODE_G)) {
		if (!selected_entities.empty())
			is_grabbed = !is_grabbed;
		else
			is_grabbed = false;
	}

	if (input::key_down(SDL_SCANCODE_F)) {
		// TODO: skip collision testing instead for simple position testing
		// because entites with NO_COLLISION flag arent selected.
		auto entity = manager->find_entity_by_position(cursor_pos);
		for (auto tmp_entity : selected_entities) {
			if (entity.lock() == tmp_entity.lock()) {
				logger::warning("Already selected entity");
				return;
			}
		}

		selected_entities.push_back(entity);
	}

	if (input::key_down(SDL_SCANCODE_R)) {
		selected_entities.clear();
		is_grabbed = false;
	}

	if (input::key_down(SDL_SCANCODE_X)) {
		if (!selected_entities.empty()) {
			for (auto selected : selected_entities) {
				manager->remove_entity(selected);
			}
			selected_entities.clear();
		}
		else {
			manager->remove_entity(manager->find_entity_by_position(cursor_pos));
		}
	}

	if (input::key_down(SDL_SCANCODE_1)) {
		if (!manager->is_entity_at_position(cursor_pos)) {
			manager->add_entity("block", uuid(), 0, cursor_pos);
		}
		else {
			logger::warning("Entity already in position");
		}
	}
	else if (input::key_down(SDL_SCANCODE_2)) {
		if (!manager->is_entity_at_position(cursor_pos)) {
			manager->add_entity("player", uuid(), 0, cursor_pos);
		}
		else {
			logger::warning("Entity already in position");
		}
	}
}

void editor_manager::free_cam_mode_update(double dt, std::shared_ptr<camera> cam) {
	is_cam_control = input::mouse_button_pressed(mouse_button::MOUSE_RIGHT);
	if (is_cam_control) {
		glm::vec3 new_pos = cursor_pos;
		float cam_speed = cam_movement_speed * dt;
		if (input::button_pressed("Run"))
			cam_speed *= 2;

		if (!(input::button_pressed("MoveUp") && input::button_pressed("MoveDown"))) {
			if (input::button_pressed("MoveUp")) {
				cam->position += cam->front * cam_speed;
			}
			else if (input::button_pressed("MoveDown")) {
				cam->position -= cam->front * cam_speed;
			}
		}

		glm::vec3 up = { 0,1,0 };
		if (!(input::button_pressed("MoveLeft") && input::button_pressed("MoveRight"))) {
			if (input::button_pressed("MoveLeft")) {
				//cursor_pos -= glm::normalize(glm::cross(cam->front, up)) * cam_speed;
				cam->position -= cam->right * cam_speed;
			}
			else if (input::button_pressed("MoveRight")) {
				//cursor_pos += glm::normalize(glm::cross(cam->front, up)) * cam_speed;
				cam->position += cam->right * cam_speed;
			}
		}

		if (input::button_pressed("IncreaseHeight")) {
			cam->position += cam->up * cam_speed;
		}
		else if (input::button_pressed("DecreaseHeight")) {
			cam->position -= cam->up * cam_speed;
		}

		cursor_pos = cam->position;

		SDL_SetRelativeMouseMode(SDL_TRUE);

		glm::vec3 camera_rotation = glm::vec3(input::get_mouse_delta().y, -input::get_mouse_delta().x, 0.0f);
		camera_rotation *= cam_rotation_speed;
		camera_rotation *= dt;
		cam->rotation += camera_rotation;

		if (camera_rotation.y > 360.0f)
			camera_rotation.y -= 360.0f;
		if (camera_rotation.y < 0.0f)
			camera_rotation.y += 360.0f;

		if (cam->rotation.x > 89.0f)
			cam->rotation.x = 89.0f;
		if (cam->rotation.x < -89.0f)
			cam->rotation.x = -89.0f;
	}
	else {
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
}

void editor_manager::update(double dt) {
	manager->cameras.set_camera("Editor");

	if (auto editor_cam = manager->cameras.get_camera("Editor").lock()) {
		switch (mode)
		{
		case editor_manager::free_cam:
			free_cam_mode_update(dt, editor_cam);
			break;
		case editor_manager::placement_cam:
			placement_cam_mode_update(dt, editor_cam);
			break;
		default:
			break;
		}
	}
}

static void draw_entity_data(std::weak_ptr<entity> _entity) {
	static const float button_width = 0.25f;
	if (auto tmp_entity = _entity.lock()) {
		ImGui::Text("ID: %llu", tmp_entity->id);
		ImGui::Text("flags: %i", tmp_entity->flags);

		if (ImGui::Button("DISABLED", { ImGui::GetWindowSize().x * button_width, 0.0f }))
			ENTITY_FLAG_TOGGLE(tmp_entity->flags, ENTITY_DISABLED);

		ImGui::SameLine();
		ImGui::Text((tmp_entity->flags & ENTITY_DISABLED) ? "ON" : "OFF");

		if (ImGui::Button("NO_COLLISION", { ImGui::GetWindowSize().x * button_width, 0.0f }))
			ENTITY_FLAG_TOGGLE(tmp_entity->flags, ENTITY_NO_COLLISION);

		ImGui::SameLine();
		ImGui::Text((tmp_entity->flags & ENTITY_NO_COLLISION) ? "ON" : "OFF");

		if (ImGui::Button("NO_CLIMB", { ImGui::GetWindowSize().x * button_width, 0.0f }))
			ENTITY_FLAG_TOGGLE(tmp_entity->flags, ENTITY_NO_CLIMB);

		ImGui::SameLine();
		ImGui::Text((tmp_entity->flags & ENTITY_NO_CLIMB) ? "ON" : "OFF");
	}
}

void editor_manager::draw() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(renderer::window);
	ImGui::NewFrame();

	static bool p_open = false;

	ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;

	/* ----- OVERLAY ----- */ {
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0.0f, 0.0f));

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
		window_flags |= ImGuiWindowFlags_NoNav;

		if (ImGui::Begin("OVERLAY", &p_open, window_flags)) {
			ImGui::Text("Resolution: %i / %i", renderer::screen_resolution_x, renderer::screen_resolution_y);
			ImGui::Text("Number of entities %i", manager->entities.size());

			ImGui::End();
		}
	}

	/* ----- CAMERA SETTINGS ----- */ {
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;

		ImGui::SetNextWindowPos(ImVec2(0, work_size.y), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(150.0f, work_size.y / 2), ImGuiCond_Always);

		if (ImGui::Begin("CAMERA SETTINGS", &p_open, window_flags)) {
			if (ImGui::Button("Placement Cam")) {
				mode = placement_cam;
			}

			if (ImGui::Button("Free Cam")) {
				mode = free_cam;
			}

			ImGui::End();
		}
	}

	/* ----- LEVEL DATA ----- */ {
		// Set Windows Flags
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;

		ImGui::SetNextWindowPos(ImVec2(work_size.x, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(300.0f, work_size.y), ImGuiCond_Always);

		if (ImGui::Begin("LEVEL DATA", &p_open, window_flags)) {
			ImGui::InputText("Level Name", &manager->name);

			if (ImGui::Button("Save")) {
				if (!manager->name.empty()) {
					manager->save();
				}
				else {
					logger::info("Name Empty\n");
				}
			}
			
			ImGui::SameLine();

			if (ImGui::Button("Load")) {
				if (!manager->name.empty()) {
					manager->load(manager->name);
				}
				else {
					logger::info("Name Empty\n");
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Clear")) {
				manager->clean();
			}

			ImGui::Separator();

			for (auto _entity : selected_entities) {
				draw_entity_data(_entity);
			}

			ImGui::End();
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}