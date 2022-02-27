#include "editor.h"

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <SDL.h>

#include <ImGuizmo.h>

#include "window.h"
#include "input.h"
#include "renderer.h"
#include "camera.h"
#include "log.h"
#include "common.h"

#include "player.h"
#include "world_entities.h"

int editor_t::read_framebuffer_pixel(int x, int y)
{
	bind_framebuffer();
	glReadBuffer(GL_COLOR_ATTACHMENT1);

	int pixel_data;
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixel_data);

	unbind_framebuffer();
	return pixel_data;
}

float editor_t::read_framebuffer_depth_pixel(int x, int y)
{
	bind_framebuffer();
	glReadBuffer(GL_DEPTH_STENCIL_ATTACHMENT);

	float pixel_data;
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pixel_data);
	glReadBuffer(GL_NONE);

	unbind_framebuffer();
	return pixel_data;
}

void editor_t::clear_selected_entities()
{
	selected_entities.clear();
}

void editor_t::select_entity(std::weak_ptr<entity_t> _entity)
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

void editor_t::add_multiselect_entity(std::weak_ptr<entity_t> _entity)
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

void editor_t::draw_entity_data(std::weak_ptr<entity_t> _entity)
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
			tmp_entity->visual_transform.position = tmp_entity->grid_pos;
		}

		// VISUAL POSITIONS
		ImGui::Separator();
		ImGui::Text("----- visual transforms -----");
		float pos_vec3f[3] = { tmp_entity->visual_transform.position.x, tmp_entity->visual_transform.position.y, tmp_entity->visual_transform.position.z };
		ImGui::DragFloat3("position", pos_vec3f, 0.03f);
		glm::vec3 new_position = { pos_vec3f[0], pos_vec3f[1], pos_vec3f[2] };
		tmp_entity->visual_transform.position = new_position;

		float rot_vec3f[3] = { tmp_entity->visual_transform.rotation.x, tmp_entity->visual_transform.rotation.y, tmp_entity->visual_transform.rotation.z };
		ImGui::DragFloat3("rotation", rot_vec3f);
		glm::vec3 new_rotation = { rot_vec3f[0], rot_vec3f[1], rot_vec3f[2] };
		tmp_entity->visual_transform.rotation = new_rotation;

		float scl_vec3f[3] = { tmp_entity->visual_transform.scale.x, tmp_entity->visual_transform.scale.y, tmp_entity->visual_transform.scale.z };
		ImGui::DragFloat3("scale", scl_vec3f, 0.03f);
		glm::vec3 new_scale = { scl_vec3f[0], scl_vec3f[1], scl_vec3f[2] };
		tmp_entity->visual_transform.scale = new_scale;
		
		if(ImGui::Button("Reset Visual Transform"))
		{
			tmp_entity->visual_transform.position = tmp_entity->grid_pos;
			tmp_entity->visual_transform.rotation = glm::vec3(0);
			tmp_entity->visual_transform.scale = glm::vec3(1);
		}

		// FLAGS
		ImGui::Separator();
		ImGui::Text("flags: %i", tmp_entity->flags);
		if (ImGui::Button("DISABLED", { ImGui::GetWindowSize().x * button_width, 0.0f }))
		{
			tmp_entity->flags.toggle(entity_flags_disabled);
		}

		ImGui::SameLine();
		ImGui::Text(tmp_entity->flags.has(entity_flags_disabled) ? "ON" : "OFF");

		if (ImGui::Button("NO_COLLISION", { ImGui::GetWindowSize().x * button_width, 0.0f }))
		{
			tmp_entity->flags.toggle(entity_flags_no_collision);
		}

		ImGui::SameLine();
		ImGui::Text(tmp_entity->flags.has(entity_flags_no_collision) ? "ON" : "OFF");

		if (ImGui::Button("NO_CLIMB", { ImGui::GetWindowSize().x * button_width, 0.0f }))
		{
			tmp_entity->flags.toggle(entity_flags_no_climb);
		}

		ImGui::SameLine();
		ImGui::Text(tmp_entity->flags.has(entity_flags_no_climb) ? "ON" : "OFF");

		ImGui::Separator();

		if(ImGui::Button("Duplicate"))
		{
			add_entity_and_init(world_t::get().get_current_chunk(), tmp_entity->extract_data());
			select_entity(world_t::get().get_current_chunk()->entities.back());
		}

		ImGui::SameLine();

		if(ImGui::Button("Move Chunk"))
		{
		}

		ImGui::Separator();

		ImGui::PopID();
	}
}

void editor_t::draw_gizmo_at_selected_entity()
{
	if (!selected_entities.empty())
	{
		ImGuizmo::SetOrthographic(false);

		float width = (float)ImGui::GetWindowWidth();
		float height = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(0, 0, renderer_t::get().screen_resolution_x, renderer_t::get().screen_resolution_y);

		if (transform_type == GRID_EDITING)
		{
			glm::vec3 average_gizmo_pos = glm::vec3(0);
			for (auto& entity : selected_entities)
			{
				average_gizmo_pos += entity.lock()->grid_pos;
			}
			average_gizmo_pos /= selected_entities.size();

			glm::mat4 transform = glm::mat4(1.0f);

			transform = glm::translate(transform, average_gizmo_pos);

			float snap_values[3] = { 1.0f, 1.0f, 1.0f };

			ImGuizmo::Manipulate(glm::value_ptr(renderer_t::get().view_matrix), glm::value_ptr(renderer_t::get().projection_matrix),
				ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::WORLD, glm::value_ptr(transform),
				nullptr, snap_values);

			auto og_pos = average_gizmo_pos;
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
							tmp_entity->visual_transform.position = tmp_entity->grid_pos;
						}
					}
				}
			}
		}
		else if (transform_type == VISUAL_EDITING)
		{
			if (auto entity = selected_entities.front().lock())
			{
				glm::vec3 average_gizmo_pos = glm::vec3(0);
				for (auto& entity : selected_entities)
				{
					average_gizmo_pos += entity.lock()->visual_transform.position;
				}
				average_gizmo_pos /= selected_entities.size();

				glm::mat4 transform = glm::mat4(1.0f);

				transform = glm::translate(transform, average_gizmo_pos);

				transform = glm::rotate(transform, glm::radians(entity->visual_transform.rotation.x), glm::vec3(1, 0, 0));
				transform = glm::rotate(transform, glm::radians(entity->visual_transform.rotation.y), glm::vec3(0, 1, 0));
				transform = glm::rotate(transform, glm::radians(entity->visual_transform.rotation.z), glm::vec3(0, 0, 1));

				transform = glm::scale(transform, entity->visual_transform.scale);

				bool snap = input_t::get().key_pressed(SDL_SCANCODE_LCTRL);
				float snap_value = 0.5f;

				if (gizmo_type == ImGuizmo::OPERATION::ROTATE)
				{
					snap_value = 45.0f;
				}

				float snap_values[3] = { snap_value, snap_value, snap_value };

				ImGuizmo::Manipulate(glm::value_ptr(renderer_t::get().view_matrix), glm::value_ptr(renderer_t::get().projection_matrix),
					(ImGuizmo::OPERATION)gizmo_type, ImGuizmo::WORLD, glm::value_ptr(transform),
					nullptr, snap ? snap_values : nullptr);

				auto og_pos = average_gizmo_pos;
				auto og_rot = entity->visual_transform.rotation;
				auto og_scl = entity->visual_transform.scale;
				if (ImGuizmo::IsUsing())
				{
					glm::vec3 pos, rot, scl;
					if (decompose_transform(transform, pos, rot, scl))
					{
						for (auto& _entity : selected_entities)
						{
							if (auto tmp_entity = _entity.lock())
							{
								auto offset_pos = pos - og_pos;
								auto offset_rot = rot - og_rot;
								auto offset_scl = scl - og_scl;

								entity->visual_transform.position += offset_pos;
								entity->visual_transform.rotation += offset_rot;

								entity->visual_transform.scale += offset_scl;
							}
						}
					}
				}
			}
		}
	}
}

void editor_t::init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window_t::get().window, renderer_t::get().context);
	ImGui_ImplOpenGL3_Init("#version 330");

	init_framebuffer();

	log_info("Initialized Editor");
}

void editor_t::shutdown()
{
	shutdown_framebuffer();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	log_info("Editor Shutdown");
}

void editor_t::handle_events()
{
	ImGui_ImplSDL2_ProcessEvent(&window_t::get().window_event);
}


void editor_t::init_framebuffer()
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

	framebuffer_quad_shader = asset_manager_t::get().load_shader_from_file("data/shaders/framebuffer.glsl");

	glUseProgram(framebuffer_quad_shader->id);

	glUniform1i(glGetUniformLocation(framebuffer_quad_shader->id, "screenTexture"), 0);
}

void editor_t::shutdown_framebuffer()
{
	glDeleteFramebuffers(1, &framebuffer_id);
	log_info("Deleted framebuffer");
}

void editor_t::bind_framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
}

void editor_t::unbind_framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void editor_t::draw_framebuffer()
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(framebuffer_quad_shader->id);
	glBindVertexArray(framebuffer_quad_vao);
	glBindTexture(GL_TEXTURE_2D, color_texture_attachment.id);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEnable(GL_DEPTH_TEST);
}


void editor_t::update(double dt)
{
	camera_manager_t::get().set_camera("Editor");

	if (!can_use_keyboard)
		return;

	if (auto cam = camera_manager_t::get().get_camera("Editor").lock())
	{
		if (!can_use_mouse)
			return;

		is_cam_control = input_t::get().mouse_button_pressed(mouse_button::MOUSE_RIGHT);
		if (is_cam_control)
		{
			float cam_speed = cam_movement_speed;
			if (input_t::get().key_pressed(SDL_SCANCODE_LSHIFT))
				cam_speed = 0.1f;
			else if (input_t::get().key_pressed(SDL_SCANCODE_LALT))
				cam_speed = 0.003f;

			cam_speed *= dt;

			if (!(input_t::get().key_pressed(SDL_SCANCODE_W) && input_t::get().key_pressed(SDL_SCANCODE_S)))
			{
				if (input_t::get().key_pressed(SDL_SCANCODE_W))
				{
					cam->position += cam->front * cam_speed;
				}
				else if (input_t::get().key_pressed(SDL_SCANCODE_S))
				{
					cam->position -= cam->front * cam_speed;
				}
			}

			glm::vec3 up = { 0,1,0 };
			if (!(input_t::get().key_pressed(SDL_SCANCODE_A) && input_t::get().key_pressed(SDL_SCANCODE_D)))
			{
				if (input_t::get().key_pressed(SDL_SCANCODE_A))
				{
					cam->position -= cam->right * cam_speed;
				}
				else if (input_t::get().key_pressed(SDL_SCANCODE_D))
				{
					cam->position += cam->right * cam_speed;
				}
			}

			if (input_t::get().key_pressed(SDL_SCANCODE_E))
			{
				cam->position += cam->up * cam_speed;
			}
			else if (input_t::get().key_pressed(SDL_SCANCODE_Q))
			{
				cam->position -= cam->up * cam_speed;
			}

			SDL_SetRelativeMouseMode(SDL_TRUE);
			SDL_SetWindowGrab(window_t::get().window, SDL_TRUE);

			glm::vec3 camera_rotation = glm::vec3(input_t::get().get_mouse_delta().y, -input_t::get().get_mouse_delta().x, 0.0f);
			camera_rotation *= cam_rotation_speed;
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
			SDL_SetWindowGrab(window_t::get().window, SDL_FALSE);

			if (input_t::get().key_down(SDL_SCANCODE_Q))
			{
				placement_mode = false;
			}

			if(input_t::get().key_pressed(SDL_SCANCODE_LCTRL))
			{
				if(input_t::get().key_down(SDL_SCANCODE_D))
				{
					if(!selected_entities.empty())
					{
						for(auto& entity : selected_entities)
						{
							if(auto tmp_entity = entity.lock())
							{
								copied_data_entities.push_back(tmp_entity->extract_data());
							}
						}

						clear_selected_entities();

						for(auto& entity_data : copied_data_entities)
						{
							add_entity_and_init(world_t::get().get_current_chunk(), entity_data);
							add_multiselect_entity(world_t::get().get_current_chunk()->entities.back());
						}

						copied_data_entities.clear();
					}
				}
			}

			glm::vec2 mouse_pos = glm::vec2(input_t::get().get_mouse_pos().x, renderer_t::get().screen_resolution_y - 1.0f - input_t::get().get_mouse_pos().y);

			if (placement_mode)
			{
				clear_selected_entities();

				auto depth_value = read_framebuffer_depth_pixel(mouse_pos.x, mouse_pos.y);

				glm::vec4 viewport = { 0,0,renderer_t::get().screen_resolution_x, renderer_t::get().screen_resolution_y };
				placement_position = glm::unProject({ mouse_pos.x, mouse_pos.y, depth_value }, renderer_t::get().view_matrix, renderer_t::get().projection_matrix, viewport);

				int entity_index = read_framebuffer_pixel(mouse_pos.x, mouse_pos.y);
				auto entity = std::weak_ptr<entity_t>();
				for (auto& chunk : world_t::get().chunks)
				{
					auto tmp = world_t::get().get_current_chunk()->find_entity_by_index(entity_index);
					if (tmp.lock())
					{
						entity = tmp;
					}
				}

				if (auto tmp_entity = entity.lock())
				{
					glm::vec3 offset = placement_position - (glm::vec3)tmp_entity->grid_pos;

					offset = shortest_vector_value(offset);

					offset = round_vec_up_to_nearest(offset);
					offset = glm::normalize(offset);

					glm::ivec3 grid_position = tmp_entity->grid_pos + vec_to_ivec(offset);
					renderer_t::get().primitive_renderer.add_wireframe_cube(grid_position, glm::vec3(1.0f), colour::green);

					if (input_t::get().mouse_button_released(mouse_button::MOUSE_LEFT))
					{
						if(!world_t::get().get_current_chunk()->is_entity_at_position(grid_position))
						{
							add_entity_and_init(world_t::get().get_current_chunk(), entity_data_t(placement_type, uuid(), entity_flags_t(), grid_position));
						}
					}
				}
			}
			else
			{
				int entity_index = read_framebuffer_pixel(mouse_pos.x, mouse_pos.y);
				auto entity = world_t::get().get_current_chunk()->find_entity_by_index(entity_index);

				static bool is_selecting = false;
				if (auto tmp_entity = entity.lock())
				{
					renderer_t::get().primitive_renderer.add_wireframe_cube(tmp_entity->grid_pos, glm::vec3(1.0f), colour::green);
					
					if(input_t::get().mouse_button_down(mouse_button::MOUSE_LEFT))
					{
						if (!input_t::get().key_pressed(SDL_SCANCODE_LSHIFT))
						{
							clear_selected_entities();
						}

						start_select = tmp_entity->grid_pos;

						is_selecting = true;
					}

					if (input_t::get().mouse_button_released(mouse_button::MOUSE_LEFT) && is_selecting)
					{
						end_select = tmp_entity->grid_pos;

						auto[end_select, start_select] = swap_for_smallest_vector(end_select, start_select);

						for(int x = start_select.x; x <= end_select.x; x++)
						{
							for(int y = start_select.y; y <= end_select.y; y++)
							{
								for(int z = start_select.z; z <= end_select.z; z++)
								{
									auto entity = world_t::get().get_current_chunk()->get_entity_at_position(glm::ivec3(x, y, z));
									if(entity.lock())
									{
										add_multiselect_entity(entity);

										is_selecting = false;
									}
								}
							}
						}
					}
				}
				else
				{
					if (input_t::get().mouse_button_down(mouse_button::MOUSE_LEFT) ||
						input_t::get().mouse_button_released(mouse_button::MOUSE_RIGHT))
					{
						is_selecting = false;
					}

					if (input_t::get().mouse_button_released(mouse_button::MOUSE_LEFT))
					{
						clear_selected_entities();
					}
				}
			}

			if (input_t::get().key_down(SDL_SCANCODE_TAB))
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

			if (input_t::get().key_down(SDL_SCANCODE_Q))
			{
				gizmo_type = -1;
			}
			else if (input_t::get().key_down(SDL_SCANCODE_W))
			{
				gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
			}
			else if (input_t::get().key_down(SDL_SCANCODE_E))
			{
				gizmo_type = ImGuizmo::OPERATION::ROTATE;
			}
			else if (input_t::get().key_down(SDL_SCANCODE_R))
			{
				gizmo_type = ImGuizmo::OPERATION::SCALE;
			}
		}
	}

	if (input_t::get().key_down(SDL_SCANCODE_X) || input_t::get().key_down(SDL_SCANCODE_DELETE))
	{
		if (!selected_entities.empty())
		{
			for (auto selected : selected_entities)
			{
				remove_entity(world_t::get().get_current_chunk(), selected);
			}
			clear_selected_entities();
		}
	}

	for (auto selected : selected_entities)
	{
		if (auto tmp_entity = selected.lock())
		{
			renderer_t::get().primitive_renderer.add_wireframe_cube(tmp_entity->grid_pos, glm::vec3(1), colour::white);

			if (tmp_entity->visual_transform.position != (glm::vec3)tmp_entity->grid_pos)
			{
				renderer_t::get().primitive_renderer.add_wireframe_cube(tmp_entity->visual_transform.position, glm::vec3(1), colour::red);
			}
		}
	}
}

void editor_t::draw()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window_t::get().window);
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	ImGuiIO& io = ImGui::GetIO();

	can_use_mouse = !io.WantCaptureMouse;
	can_use_keyboard = !io.WantCaptureKeyboard;
	
	if(!is_cam_control)
	{
		draw_gizmo_at_selected_entity();
	}

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
			ImGui::Text("Resolution: %i / %i", renderer_t::get().screen_resolution_x, renderer_t::get().screen_resolution_y);

			ImGui::Text("No. of chunks %i", world_t::get().chunks.size());

			int num_of_entities = 0;
			for(auto& chunk : world_t::get().chunks)
			{
				num_of_entities += chunk->entities.size();
			}

			ImGui::Text("No. of entites overall %i", num_of_entities);

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

		if (ImGui::Begin("LEVEL DATA", &p_open, window_flags))
		{
			ImGui::InputText("Chunk Name", &world_t::get().get_current_chunk()->name);

			if (ImGui::Button("Save"))
			{
				world_t::get().save();
			}
			
			ImGui::SameLine();

			if (ImGui::Button("Load"))
			{
				renderer_t::get().instance_containers.clear();
				world_t::get().load();
				renderer_t::get().construct_all_instance_buffers();
			}

			if(ImGui::BeginListBox("Chunks", {-1, 0}))
			{
				for(int i = 0; i < world_t::get().chunks.size(); i++)
				{
					bool is_selected = false;

					is_selected = (world_t::get().current_chunk == i);

					ImGui::PushID(world_t::get().chunks[i].get());
					if(ImGui::Selectable(world_t::get().chunks[i]->name.c_str(), is_selected))
					{
						world_t::get().current_chunk = i;
					}

					if(is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
					else
					{
						if(world_t::get().chunks[i]->entities.size() == 0 && world_t::get().chunks[i]->name.empty())
						{
							world_t::get().delete_chunk(i);
						}
					}

					ImGui::PopID();
				}
				ImGui::ListBoxFooter();
			}

			if (ImGui::Button("Clear"))
			{
				world_t::get().get_current_chunk()->clear_data();
			}

			ImGui::SameLine();

			if (ImGui::Button("Delete"))
			{
				world_t::get().delete_chunk(world_t::get().current_chunk);
			}

			ImGui::SameLine();

			if (ImGui::Button("New"))
			{
				world_t::get().current_chunk = world_t::get().new_chunk();
			}

			ImGui::Text("No. Of Entities %i", world_t::get().get_current_chunk()->entities.size());

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
					placement_type = "";
					placement_mode = false;
				}
			}

			if (ImGui::CollapsingHeader("Entities"))
			{
				if (ImGui::Button("Block"))
				{
					placement_type = "block";
					placement_mode = true;
				}

				ImGui::SameLine();
				if (ImGui::Button("Player Spawn"))
				{
					placement_type = "player_spawn";
					placement_mode = true;
				}
			}

			ImGui::End();
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
