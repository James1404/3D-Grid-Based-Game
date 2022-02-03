#include "editor.h"

#include <SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <string>

#include "input.h"
#include "renderer.h"
#include "entity.h"
#include "camera.h"
#include "log.h"

#include "player.h"
#include "world_entities.h"

void editor_manager::init(entity_manager_t& _entity_manager)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(renderer::window, renderer::context);
	ImGui_ImplOpenGL3_Init("#version 330");

	entity_manager = &_entity_manager;

	init_framebuffer();

	log_info("INITIALIZED EDITOR");
}

void editor_manager::clean()
{
	shutdown_framebuffer();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	log_info("-----------------\n");
	log_info("EDITOR CLEANED UP\n");
}

void editor_manager::init_framebuffer()
{
	// Initialize framebuffer
	glGenFramebuffers(1, &framebuffer_id);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

	int resolution_x = 1280, resolution_y = 720;

	// COLOR TEXTURE
	glGenTextures(1, &color_texture_attachment.id);
	glBindTexture(GL_TEXTURE_2D, color_texture_attachment.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution_x, resolution_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_attachment.id, 0);

	// ENTITY ID TEXTURE
	glGenTextures(1, &id_texture_attachment.id);
	glBindTexture(GL_TEXTURE_2D, id_texture_attachment.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, resolution_x, resolution_y, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, id_texture_attachment.id, 0);

	// DEPTH TEXTURE
	glGenTextures(1, &depth_texture_attachment.id);
	glBindTexture(GL_TEXTURE_2D, depth_texture_attachment.id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, resolution_x, resolution_y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_texture_attachment.id, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		log_error("Framebuffer is not complete");
	}

	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	float quad_vertices[] =
	{
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &framebuffer_quad_vao);
	glGenBuffers(1, &framebuffer_quad_vbo);
	glBindVertexArray(framebuffer_quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, framebuffer_quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	framebuffer_quad_shader = renderer::asset_manager.load_shader_from_file("data/shaders/framebuffer.glsl");

	glUseProgram(framebuffer_quad_shader->id);

	glUniform1i(glGetUniformLocation(framebuffer_quad_shader->id, "screenTexture"), 0);
}

void editor_manager::shutdown_framebuffer()
{
	glDeleteFramebuffers(1, &framebuffer_id);
	log_info("Deleted framebuffer");
}

void editor_manager::bind_framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
}

void editor_manager::unbind_framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void editor_manager::draw_framebuffer()
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(framebuffer_quad_shader->id);
	glBindVertexArray(framebuffer_quad_vao);
	glBindTexture(GL_TEXTURE_2D, color_texture_attachment.id);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);
}

int editor_manager::read_framebuffer_pixel(int x, int y)
{
	bind_framebuffer();
	glReadBuffer(GL_COLOR_ATTACHMENT1);

	int pixel_data;
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixel_data);

	unbind_framebuffer();
	return pixel_data;
}

void editor_manager::move_cursor(glm::ivec3 _vel)
{
	if (_vel == glm::ivec3(0))
		return;

	cursor_pos += _vel;

	if (is_grabbed)
	{
		for (auto selected : selected_entities)
		{
			if (auto tmp_entity = selected.lock())
			{
				tmp_entity->grid_pos += _vel;
				tmp_entity->visual_pos = tmp_entity->grid_pos;
			}
		}
	}
}

void editor_manager::select_entity(std::weak_ptr<entity> _entity)
{
	for (auto tmp_entity : selected_entities)
	{
		if (_entity.lock() == tmp_entity.lock())
		{
			return;
		}
	}

	clear_selected_entities();
	selected_entities.push_back(_entity);
}

void editor_manager::add_multiselect_entity(std::weak_ptr<entity> _entity)
{
	for (auto tmp_entity : selected_entities)
	{
		if (_entity.lock() == tmp_entity.lock())
		{
			return;
		}
	}

	selected_entities.push_back(_entity);
}

void editor_manager::clear_selected_entities()
{
	selected_entities.clear();
}

void editor_manager::placement_cam_mode_update(double dt, input_manager_t& input_manager, std::shared_ptr<camera_t> cam)
{
	cursor_pos = vec_to_ivec(cursor_pos);
	glm::ivec3 vel{ 0 };
	if (!(input_manager.key_down(SDL_SCANCODE_W) && input_manager.key_down(SDL_SCANCODE_S)))
	{
		if (input_manager.key_down(SDL_SCANCODE_W))
		{
			vel = { 0,0, -1 };
		}
		else if (input_manager.key_down(SDL_SCANCODE_S))
		{
			vel = { 0,0, 1 };
		}
	}

	if (!(input_manager.key_down(SDL_SCANCODE_A) && input_manager.key_down(SDL_SCANCODE_D)))
	{
		if (input_manager.key_down(SDL_SCANCODE_A))
		{
			vel = { -1,0,0 };
		}
		else if (input_manager.key_down(SDL_SCANCODE_D))
		{
			vel = { 1,0,0 };
		}
	}

	if (input_manager.key_down(SDL_SCANCODE_E))
	{
		vel = { 0,1,0 };
	}
	else if (input_manager.key_down(SDL_SCANCODE_Q))
	{
		vel = { 0,-1,0 };
	}

	glm::vec3 offset = glm::vec3(0, 6, 5);
	cam->position = ((glm::vec3)cursor_pos + offset);

	cam->rotation.x = -50;
	cam->rotation.y = -90;

	move_cursor(vel);

	//renderer::debug::draw_box_wireframe(cursor_pos, glm::vec3(1), colour::white);

	if (input_manager.key_down(SDL_SCANCODE_F))
	{
		// TODO: skip collision testing instead for simple position testing
		// because entites with NO_COLLISION flag arent selected.
		add_multiselect_entity(entity_manager->find_entity_by_position(cursor_pos));
	}

	if (input_manager.key_down(SDL_SCANCODE_1))
	{
		if (!entity_manager->is_entity_at_position(cursor_pos))
		{
			entity_manager->add_entity("block", uuid(), 0, cursor_pos);
		}
	}
	else if (input_manager.key_down(SDL_SCANCODE_2))
	{
		if (!entity_manager->is_entity_at_position(cursor_pos))
		{
			entity_manager->add_entity("player", uuid(), 0, cursor_pos);
		}
		else
		{
			log_warning("Entity already in position");
		}
	}
}

void editor_manager::free_cam_mode_update(double dt, input_manager_t& input_manager, std::shared_ptr<camera_t> cam)
{
	if (!can_use_mouse)
		return;

	is_cam_control = input_manager.mouse_button_pressed(mouse_button::MOUSE_RIGHT);
	if (is_cam_control)
	{
		glm::vec3 new_pos = cursor_pos;
		float cam_speed = cam_movement_speed;
		if (input_manager.key_pressed(SDL_SCANCODE_LSHIFT))
			cam_speed = 0.3f;
		else if (input_manager.key_pressed(SDL_SCANCODE_LALT))
			cam_speed = 0.005f;

		cam_speed *= dt;

		if (!(input_manager.key_pressed(SDL_SCANCODE_W) && input_manager.key_pressed(SDL_SCANCODE_S)))
		{
			if (input_manager.key_pressed(SDL_SCANCODE_W)) {
				cam->position += cam->front * cam_speed;
			}
			else if (input_manager.key_pressed(SDL_SCANCODE_S)) {
				cam->position -= cam->front * cam_speed;
			}
		}

		glm::vec3 up = { 0,1,0 };
		if (!(input_manager.key_pressed(SDL_SCANCODE_A) && input_manager.key_pressed(SDL_SCANCODE_D)))
		{
			if (input_manager.key_pressed(SDL_SCANCODE_A))
			{
				//cursor_pos -= glm::normalize(glm::cross(cam->front, up)) * cam_speed;
				cam->position -= cam->right * cam_speed;
			}
			else if (input_manager.key_pressed(SDL_SCANCODE_D))
			{
				//cursor_pos += glm::normalize(glm::cross(cam->front, up)) * cam_speed;
				cam->position += cam->right * cam_speed;
			}
		}

		if (input_manager.key_pressed(SDL_SCANCODE_E))
		{
			cam->position += cam->up * cam_speed;
		}
		else if (input_manager.key_pressed(SDL_SCANCODE_Q))
		{
			cam->position -= cam->up * cam_speed;
		}

		cursor_pos = cam->position;

		SDL_SetRelativeMouseMode(SDL_TRUE);

		glm::vec3 camera_rotation = glm::vec3(input_manager.get_mouse_delta().y, -input_manager.get_mouse_delta().x, 0.0f);
		camera_rotation *= cam_rotation_speed;
		camera_rotation *= dt;
		cam->rotation += camera_rotation;

		if (cam->rotation.y > 360.0f)
			cam->rotation.y -= 360.0f;
		if (cam->rotation.y < 0.0f)
			cam->rotation.y += 360.0f;

		if (cam->rotation.x > 89.0f)
			cam->rotation.x = 89.0f;
		if (cam->rotation.x < -89.0f)
			cam->rotation.x = -89.0f;
	}
	else
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);

		if (input_manager.mouse_button_down(mouse_button::MOUSE_LEFT))
		{
			glm::vec2 mouse_pos = glm::vec2(input_manager.get_mouse_pos().x, renderer::screen_resolution_y - 1.0f - input_manager.get_mouse_pos().y);

			int entity_index = read_framebuffer_pixel(mouse_pos.x, mouse_pos.y);

			auto entity = entity_manager->find_entity_by_index(entity_index);
			if(entity.lock())
			{
				if (input_manager.key_pressed(SDL_SCANCODE_LSHIFT))
				{
					add_multiselect_entity(entity);
				}
				else
				{
					select_entity(entity);
				}
			}
			else
			{
				clear_selected_entities();
			}
		}
	}
}

void editor_manager::update(double dt, input_manager_t& input_manager, camera_manager_t& camera_manager)
{
	camera_manager.set_camera("Editor");

	if (!can_use_keyboard)
		return;

	if (auto editor_cam = camera_manager.get_camera("Editor").lock())
	{
		if (input_manager.key_down(SDL_SCANCODE_TAB))
		{
			if (mode == editor_mode::free_cam)
			{
				mode = editor_mode::placement_cam;
			}
			else if (mode == editor_mode::placement_cam)
			{
				mode = editor_mode::free_cam;
			}
		}

		switch (mode)
		{
		case editor_mode::free_cam:
			free_cam_mode_update(dt, input_manager, editor_cam);
			break;
		case editor_mode::placement_cam:
			placement_cam_mode_update(dt, input_manager, editor_cam);
			break;
		default:
			break;
		}
	}

	if (input_manager.key_down(SDL_SCANCODE_G))
	{
		if (!selected_entities.empty())
			is_grabbed = !is_grabbed;
		else
			is_grabbed = false;
	}

	if (input_manager.key_down(SDL_SCANCODE_R))
	{
		clear_selected_entities();
		is_grabbed = false;
	}

	if (input_manager.key_down(SDL_SCANCODE_X))
	{
		if (!selected_entities.empty())
		{
			for (auto selected : selected_entities)
			{
				entity_manager->remove_entity(selected);
			}
			clear_selected_entities();
		}
		else
		{
			entity_manager->remove_entity(entity_manager->find_entity_by_position(cursor_pos));
		}
	}

}

static void draw_entity_data(std::weak_ptr<entity> _entity)
{
	static const float button_width = 0.25f;
	if (auto tmp_entity = _entity.lock())
	{
		ImGui::PushID(tmp_entity.get());

		ImGui::Text("Index: %i", tmp_entity->index);
		ImGui::Text("ID: %llu", tmp_entity->id);
		ImGui::Text("flags: %i", tmp_entity->flags);

		int vec3i[3] = { tmp_entity->grid_pos.x, tmp_entity->grid_pos.y, tmp_entity->grid_pos.z };
		ImGui::DragInt3("grid pos", vec3i, 0.03f);
		glm::ivec3 new_grid_pos = { vec3i[0], vec3i[1], vec3i[2] };
		if (new_grid_pos != tmp_entity->grid_pos)
		{
			tmp_entity->grid_pos = new_grid_pos;
			tmp_entity->visual_pos = tmp_entity->grid_pos;
		}

		float vec3f[3] = { tmp_entity->visual_pos.x, tmp_entity->visual_pos.y, tmp_entity->visual_pos.z };
		ImGui::DragFloat3("visual pos", vec3f, 0.03f);
		glm::vec3 new_visual_pos = { vec3f[0], vec3f[1], vec3f[2] };
		tmp_entity->visual_pos = new_visual_pos;

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

		ImGui::Separator();

		//renderer::debug::draw_box_wireframe(tmp_entity->grid_pos, glm::vec3(1), colour::white);

		ImGui::PopID();
	}
}

void editor_manager::draw_primitives()
{
	for (auto selected : selected_entities)
	{
		if (auto tmp_entity = selected.lock())
		{
			renderer::debug::draw_box_wireframe(tmp_entity->grid_pos, glm::vec3(1), colour::white);
		}
	}
}

void editor_manager::draw(input_manager_t& input_manager)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(renderer::window);
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();

	can_use_mouse = !io.WantCaptureMouse;
	can_use_keyboard = !io.WantCaptureKeyboard;

	static bool p_open = false;

	ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;

	/* ----- OVERLAY ----- */
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0.0f, 0.0f));

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
		window_flags |= ImGuiWindowFlags_NoNav;

		if (ImGui::Begin("OVERLAY", &p_open, window_flags))
		{
			ImGui::Text("Resolution: %i / %i", renderer::screen_resolution_x, renderer::screen_resolution_y);
			ImGui::Text("Number of entities %i", entity_manager->entities.size());

			ImGui::End();
		}
	}

	/* ----- CAMERA SETTINGS ----- */
	{
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;

		ImGui::SetNextWindowPos(ImVec2(0, work_size.y), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(300.0f, work_size.y / 2), ImGuiCond_Always);

		static int item_current_idx = 0;
		if (ImGui::Begin("ENTITY LIST", &p_open, window_flags))
		{
			if (ImGui::BeginListBox("Entities", {-1, -1}))
			{
				for (const auto& entity : entity_manager->entities)
				{
					bool is_selected = false;
					if (!selected_entities.empty())
					{
						is_selected = ((selected_entities.front().lock()) &&
							(selected_entities.front().lock()->id == entity->id));
					}

					ImGui::PushID(entity->id);
					if (ImGui::Selectable(entity->name.c_str(), is_selected))
					{
						select_entity(entity);
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();

					ImGui::PopID();
				}
				ImGui::ListBoxFooter();
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
			ImGui::InputText("Level Name", &entity_manager->name);

			if (ImGui::Button("Save")) {
				if (!entity_manager->name.empty()) {
					entity_manager->save();
				}
				else {
					log_warning("Name Empty\n");
				}
			}
			
			ImGui::SameLine();

			if (ImGui::Button("Load"))
			{
				if (!entity_manager->name.empty())
				{
					entity_manager->load(entity_manager->name);
				}
				else
				{
					log_warning("Name Empty\n");
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Clear"))
			{
				entity_manager->clear_data();
			}

			ImGui::Separator();

			for (auto _entity : selected_entities)
			{
				draw_entity_data(_entity);
			}
			
			ImGui::End();
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
