#include "editor.h"

#include <SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <ImGuizmo.h>

#include <string>

#include "window.h"
#include "input.h"
#include "renderer.h"
#include "entity.h"
#include "camera.h"
#include "log.h"
#include "common.h"

#include "player.h"
#include "world_entities.h"

static entity_manager_t* entity_manager;

//static glm::vec3 cursor_pos{ 0 };

static std::vector<std::weak_ptr<entity>> selected_entities;

enum class editor_mode {
	placement_cam = 0,
	free_cam
};

static editor_mode mode = editor_mode::free_cam;
static const float cam_movement_speed = 0.01f;
static const float cam_rotation_speed = 0.03f;
static bool is_cam_control = false;

static bool can_use_keyboard = true;
static bool can_use_mouse = true;

static int gizmo_type = -1;

static int read_framebuffer_pixel(int x, int y)
{
	bind_editor_framebuffer();
	glReadBuffer(GL_COLOR_ATTACHMENT1);

	int pixel_data;
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixel_data);

	unbind_editor_framebuffer();
	return pixel_data;
}

static float read_framebuffer_depth_pixel(int x, int y)
{
	bind_editor_framebuffer();
	glReadBuffer(GL_DEPTH_STENCIL_ATTACHMENT);

	float pixel_data;
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pixel_data);
	glReadBuffer(GL_NONE);

	unbind_editor_framebuffer();
	return pixel_data;
}

static void clear_selected_entities()
{
	selected_entities.clear();
}

static void select_entity(std::weak_ptr<entity> _entity)
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

static void add_multiselect_entity(std::weak_ptr<entity> _entity)
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

static void draw_entity_data(std::weak_ptr<entity> _entity)
{
	static const float button_width = 0.25f;
	if (auto tmp_entity = _entity.lock())
	{
		ImGui::PushID(tmp_entity.get());

		ImGui::Text("Index: %i", tmp_entity->index);
		ImGui::Text("ID: %llu", tmp_entity->id);

		// GRID POSITIONS
		ImGui::Separator();
		ImGui::Text("----- grid transforms -----");
		int vec3i[3] = { tmp_entity->grid_pos.x, tmp_entity->grid_pos.y, tmp_entity->grid_pos.z };
		ImGui::DragInt3("grid pos", vec3i, 0.03f);
		glm::ivec3 new_grid_pos = { vec3i[0], vec3i[1], vec3i[2] };
		if (new_grid_pos != tmp_entity->grid_pos)
		{
			tmp_entity->grid_pos = new_grid_pos;
			tmp_entity->visual_pos = tmp_entity->grid_pos;
		}

		// VISUAL POSITIONS
		ImGui::Separator();
		ImGui::Text("----- visual transforms -----");
		float pos_vec3f[3] = { tmp_entity->visual_pos.x, tmp_entity->visual_pos.y, tmp_entity->visual_pos.z };
		ImGui::DragFloat3("position", pos_vec3f, 0.03f);
		glm::vec3 new_visual_pos = { pos_vec3f[0], pos_vec3f[1], pos_vec3f[2] };
		tmp_entity->visual_pos = new_visual_pos;

		float rot_vec3f[3] = { tmp_entity->visual_rotation.x, tmp_entity->visual_rotation.y, tmp_entity->visual_rotation.z };
		ImGui::DragFloat3("rotation", rot_vec3f);
		glm::vec3 new_visual_rot = { rot_vec3f[0], rot_vec3f[1], rot_vec3f[2] };
		tmp_entity->visual_rotation = new_visual_rot;

		float scl_vec3f[3] = { tmp_entity->visual_scale.x, tmp_entity->visual_scale.y, tmp_entity->visual_scale.z };
		ImGui::DragFloat3("scale", scl_vec3f, 0.03f);
		glm::vec3 new_visual_scl = { scl_vec3f[0], scl_vec3f[1], scl_vec3f[2] };
		tmp_entity->visual_scale = new_visual_scl;

		// FLAGS
		ImGui::Separator();
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

		ImGui::Separator();

		ImGui::PopID();
	}
}

enum transform_type_t
{
	GRID_EDITING,
	VISUAL_EDITING
};

static transform_type_t transform_type = GRID_EDITING;

static void draw_gizmo_at_selected_entity()
{
	if (!selected_entities.empty())
	{
		if (auto entity = selected_entities.front().lock())
		{
			ImGuizmo::SetOrthographic(false);
			//ImGuizmo::SetDrawlist();

			// TODO: mouse cursor gets locked if it goes over gizmo when controlling camera.

			float width = (float)ImGui::GetWindowWidth();
			float height = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(0, 0, screen_resolution_x, screen_resolution_y);

			if (transform_type == GRID_EDITING)
			{
				glm::mat4 transform = glm::mat4(1.0f);

				transform = glm::translate(transform, (glm::vec3)entity->grid_pos);

				float snap_values[3] = { 1.0f, 1.0f, 1.0f };

				ImGuizmo::Manipulate(glm::value_ptr(view_matrix), glm::value_ptr(projection_matrix),
					ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::WORLD, glm::value_ptr(transform),
					nullptr, snap_values);

				auto og_pos = entity->grid_pos;
				if (ImGuizmo::IsUsing())
				{
					glm::vec3 pos, rot, scl;
					if (decompose_transform(transform, pos, rot, scl))
					{
						for (auto& _entity : selected_entities)
						{
							if (auto tmp_entity = _entity.lock())
							{
								auto offset = pos - (glm::vec3)og_pos;
								tmp_entity->grid_pos += vec_to_ivec(offset);
								tmp_entity->visual_pos = tmp_entity->grid_pos;
							}
						}
					}
				}
			}
			else if (transform_type == VISUAL_EDITING)
			{
				glm::mat4 transform = glm::mat4(1.0f);

				transform = glm::translate(transform, entity->visual_pos);

				transform = glm::rotate(transform, glm::radians(entity->visual_rotation.x), glm::vec3(1, 0, 0));
				transform = glm::rotate(transform, glm::radians(entity->visual_rotation.y), glm::vec3(0, 1, 0));
				transform = glm::rotate(transform, glm::radians(entity->visual_rotation.z), glm::vec3(0, 0, 1));

				transform = glm::scale(transform, entity->visual_scale);

				bool snap = input_key_pressed(SDL_SCANCODE_LCTRL);
				float snap_value = 0.5f;

				if (gizmo_type == ImGuizmo::OPERATION::ROTATE)
				{
					snap_value = 45.0f;
				}

				float snap_values[3] = { snap_value, snap_value, snap_value };

				ImGuizmo::Manipulate(glm::value_ptr(view_matrix), glm::value_ptr(projection_matrix),
					(ImGuizmo::OPERATION)gizmo_type, ImGuizmo::WORLD, glm::value_ptr(transform),
					nullptr, snap ? snap_values : nullptr);

				auto og_pos = entity->visual_pos;
				auto og_rot = entity->visual_rotation;
				auto og_scl = entity->visual_scale;
				if (ImGuizmo::IsUsing())
				{
					glm::vec3 pos, rot, scl;
					if (decompose_transform(transform, pos, rot, scl))
					{
						for (auto& _entity : selected_entities)
						{
							if (auto tmp_entity = _entity.lock())
							{
								auto offset = pos - og_pos;

								entity->visual_pos += offset;

								//auto delta_rot = rot - entity->visual_rotation;
								entity->visual_rotation = rot;
								//entity->visual_rotation += delta_rot;

								entity->visual_scale = scl;
							}
						}
					}
				}
			}
		}
	}
}

void init_editor(entity_manager_t& _entity_manager)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 330");

	entity_manager = &_entity_manager;

	init_editor_framebuffer();

	log_info("INITIALIZED EDITOR");
}

void shutdown_editor()
{
	shutdown_editor_framebuffer();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	log_info("-----------------\n");
	log_info("EDITOR CLEANED UP\n");
}

void handle_editor_events()
{
	ImGui_ImplSDL2_ProcessEvent(&window_event);
}

texture_t depth_texture_attachment;
texture_t color_texture_attachment;
texture_t id_texture_attachment;

unsigned int framebuffer_quad_vao, framebuffer_quad_vbo;
std::shared_ptr<shader_t> framebuffer_quad_shader;

unsigned int framebuffer_id;

void init_editor_framebuffer()
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

	framebuffer_quad_shader = asset_manager.load_shader_from_file("data/shaders/framebuffer.glsl");

	glUseProgram(framebuffer_quad_shader->id);

	glUniform1i(glGetUniformLocation(framebuffer_quad_shader->id, "screenTexture"), 0);
}

void shutdown_editor_framebuffer()
{
	glDeleteFramebuffers(1, &framebuffer_id);
	log_info("Deleted framebuffer");
}

void bind_editor_framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
}

void unbind_editor_framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_editor_framebuffer()
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(framebuffer_quad_shader->id);
	glBindVertexArray(framebuffer_quad_vao);
	glBindTexture(GL_TEXTURE_2D, color_texture_attachment.id);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);
}

static bool placement_mode = false;
static glm::vec3 placement_position{ 0 };
enum class entity_type
{
	NONE = 0,
	PLAYER,
	BLOCK
};
static entity_type placement_type = entity_type::NONE;

static void free_cam_mode_update(double dt, std::shared_ptr<camera_t> cam)
{
	if (!can_use_mouse)
		return;

	is_cam_control = input_mouse_button_pressed(mouse_button::MOUSE_RIGHT);
	if (is_cam_control)
	{
		float cam_speed = cam_movement_speed;
		if (input_key_pressed(SDL_SCANCODE_LSHIFT))
			cam_speed = 0.1f;
		else if (input_key_pressed(SDL_SCANCODE_LALT))
			cam_speed = 0.003f;

		cam_speed *= dt;

		if (!(input_key_pressed(SDL_SCANCODE_W) && input_key_pressed(SDL_SCANCODE_S)))
		{
			if (input_key_pressed(SDL_SCANCODE_W)) {
				cam->position += cam->front * cam_speed;
			}
			else if (input_key_pressed(SDL_SCANCODE_S)) {
				cam->position -= cam->front * cam_speed;
			}
		}

		glm::vec3 up = { 0,1,0 };
		if (!(input_key_pressed(SDL_SCANCODE_A) && input_key_pressed(SDL_SCANCODE_D)))
		{
			if (input_key_pressed(SDL_SCANCODE_A))
			{
				//cursor_pos -= glm::normalize(glm::cross(cam->front, up)) * cam_speed;
				cam->position -= cam->right * cam_speed;
			}
			else if (input_key_pressed(SDL_SCANCODE_D))
			{
				//cursor_pos += glm::normalize(glm::cross(cam->front, up)) * cam_speed;
				cam->position += cam->right * cam_speed;
			}
		}

		if (input_key_pressed(SDL_SCANCODE_E))
		{
			cam->position += cam->up * cam_speed;
		}
		else if (input_key_pressed(SDL_SCANCODE_Q))
		{
			cam->position -= cam->up * cam_speed;
		}

		SDL_SetRelativeMouseMode(SDL_TRUE);
		SDL_SetWindowGrab(window, SDL_TRUE);
		//SDL_WarpMouseInWindow(window, window_size_x / 2, window_size_y / 2);

		glm::vec3 camera_rotation = glm::vec3(input_get_mouse_delta().y, -input_get_mouse_delta().x, 0.0f);
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
		SDL_SetWindowGrab(window, SDL_FALSE);

		if (input_key_down(SDL_SCANCODE_Q))
		{
			placement_mode = false;
		}

		glm::vec2 mouse_pos = glm::vec2(input_get_mouse_pos().x, screen_resolution_y - 1.0f - input_get_mouse_pos().y);

		if (placement_mode)
		{
			clear_selected_entities();

			auto depth_value = read_framebuffer_depth_pixel(mouse_pos.x, mouse_pos.y);

			glm::vec4 viewport = { 0,0,screen_resolution_x, screen_resolution_y };
			placement_position = glm::unProject({ mouse_pos.x, mouse_pos.y, depth_value }, view_matrix, projection_matrix, viewport);

			int entity_index = read_framebuffer_pixel(mouse_pos.x, mouse_pos.y);
			auto entity = entity_manager->find_entity_by_index(entity_index);
			if (auto tmp_entity = entity.lock())
			{
				glm::vec3 offset = placement_position - (glm::vec3)tmp_entity->grid_pos;

				offset = shortest_vector_value(offset);

				//log_info("offset: ", offset.x, ", ", offset.y, ", ", offset.z);

				offset = round_vec_up_to_nearest(offset);
				offset = glm::normalize(offset);

				//log_info("offset: ", offset.x, ", ", offset.y, ", ", offset.z);

				glm::ivec3 grid_position = tmp_entity->grid_pos + vec_to_ivec(offset);
				add_primitive_wireframe_cube(grid_position, glm::vec3(1.0f), colour::green);

				if (input_mouse_button_down(mouse_button::MOUSE_LEFT))
				{
					switch (placement_type)
					{
					case entity_type::PLAYER:
						entity_manager->add_entity("player", uuid(), 0, grid_position);
						break;
					case entity_type::BLOCK:
						entity_manager->add_entity("block", uuid(), 0, grid_position);
						break;
					}
				}
			}
		}
		else
		{
			int entity_index = read_framebuffer_pixel(mouse_pos.x, mouse_pos.y);
			auto entity = entity_manager->find_entity_by_index(entity_index);

			if (auto tmp_entity = entity.lock())
			{
				add_primitive_wireframe_cube(tmp_entity->grid_pos, glm::vec3(1.0f), colour::green);

				if (input_mouse_button_down(mouse_button::MOUSE_LEFT))
				{
					if (input_key_pressed(SDL_SCANCODE_LSHIFT))
					{
						add_multiselect_entity(entity);
					}
					else
					{
						clear_selected_entities();
						select_entity(entity);
					}
				}
			}
			else
			{
				if (!input_key_pressed(SDL_SCANCODE_LSHIFT))
				{
					if (input_mouse_button_down(mouse_button::MOUSE_LEFT))
					{
						clear_selected_entities();
					}
				}
			}
		}

		if (input_key_down(SDL_SCANCODE_TAB))
		{
			if (transform_type == GRID_EDITING)
			{
				transform_type = VISUAL_EDITING;
			}
			else if (transform_type == VISUAL_EDITING)
			{
				transform_type = GRID_EDITING;
			}
		}

		if (input_key_down(SDL_SCANCODE_Q))
		{
			gizmo_type = -1;
		}
		else if (input_key_down(SDL_SCANCODE_W))
		{
			gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
		}
		else if (input_key_down(SDL_SCANCODE_E))
		{
			gizmo_type = ImGuizmo::OPERATION::ROTATE;
		}
		else if (input_key_down(SDL_SCANCODE_R))
		{
			gizmo_type = ImGuizmo::OPERATION::SCALE;
		}
	}
}

void update_editor(double dt)
{
	set_camera("Editor");

	if (!can_use_keyboard)
		return;

	if (auto editor_cam = get_camera("Editor").lock())
	{
		free_cam_mode_update(dt, editor_cam);
	}

	if (input_key_down(SDL_SCANCODE_X) || input_key_down(SDL_SCANCODE_DELETE))
	{
		if (!selected_entities.empty())
		{
			for (auto selected : selected_entities)
			{
				entity_manager->remove_entity(selected);
			}
			clear_selected_entities();
		}
	}

	for (auto selected : selected_entities)
	{
		if (auto tmp_entity = selected.lock())
		{
			add_primitive_wireframe_cube(tmp_entity->grid_pos, glm::vec3(1), colour::white);

			if (tmp_entity->visual_pos != (glm::vec3)tmp_entity->grid_pos)
			{
				add_primitive_wireframe_cube(tmp_entity->visual_pos, glm::vec3(1), colour::red);
			}
		}
	}
}

void draw_editor()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImGuiIO& io = ImGui::GetIO();

	can_use_mouse = !io.WantCaptureMouse;
	can_use_keyboard = !io.WantCaptureKeyboard;
	
	draw_gizmo_at_selected_entity();

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
			ImGui::Text("Resolution: %i / %i", screen_resolution_x, screen_resolution_y);
			ImGui::Text("Number of entities %i", entity_manager->entities.size());

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

			if(!selected_entities.empty())
			{
				draw_entity_data(selected_entities.front());
			}
			
			ImGui::End();
		}
	}

	/* NEW ENTITES */ {
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;

		ImGui::SetNextWindowPos(ImVec2(0, work_size.y), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(300.0f, work_size.y / 2), ImGuiCond_Always);

		if (ImGui::Begin("NEW ENTITES", &p_open, window_flags))
		{
			ImGui::Text("Create new entities");

			if (placement_mode)
			{
				if (ImGui::Button("Cancel"))
				{
					placement_type = entity_type::NONE;
					placement_mode = false;
				}
			}

			if (ImGui::CollapsingHeader("Blocks"))
			{
				if (ImGui::Button("Block"))
				{
					placement_type = entity_type::BLOCK;
					placement_mode = true;
				}

				ImGui::SameLine();
				if (ImGui::Button("Player"))
				{
					placement_type = entity_type::PLAYER;
					placement_mode = true;
				}
			}

			if (ImGui::CollapsingHeader("Nodes"))
			{
				if (ImGui::Button("Player Spawn"))
				{

				}
			}
			ImGui::End();
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
