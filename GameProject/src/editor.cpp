#include "editor.h"

#ifdef _DEBUG
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <string>

#include "scene.h"
#include "input.h"
#include "renderer.h"
#include "scene.h"
#include "entity.h"
#include "player.h"
#include "camera.h"

float camera_zoom_speed = .001f;
struct editor_camera : public camera::camera_interface {
	void update(double dt) override {
		view = renderer::view;

		if (input::mouse_button_pressed(input::MOUSE_RIGHT)) {
			//  TODO: convert mouse position to world space
			//	which will fix panning speed.
			glm::vec2 mouseDelta = { -input::get_mouse_delta().x, input::get_mouse_delta().y };
			view = glm::translate(view, { mouseDelta, 0.0f });
		}

		// TODO: it zooms into the center of the game, not the center of the camera
		// TODO: use mouse wheel not buttons
		float zoom = 1;
		if (input::key_pressed(SDL_SCANCODE_Q)) { zoom -= camera_zoom_speed * dt; }
		if (input::key_pressed(SDL_SCANCODE_E)) { zoom += camera_zoom_speed * dt; }

		view = glm::scale(view, { zoom, zoom, 0 });
	}
};

void editor::init() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(renderer::window, renderer::context);
	ImGui_ImplOpenGL3_Init("#version 330");

	current_level.init();

	camera::register_camera("Editor", std::make_shared<editor_camera>());
}

glm::vec2 velocity;
float speed = 2;

static std::shared_ptr<obstacle_entity> current_obstacle = nullptr;
static std::shared_ptr<path_node> current_path_node = nullptr;
static std::shared_ptr<sprite_entity> current_sprite = nullptr;
static std::shared_ptr<enemy_entity> current_enemy = nullptr;
static std::shared_ptr<cutscene> current_cutscene = nullptr;

void editor::clear_selected() {
	current_obstacle = nullptr;
	current_path_node = nullptr;
	current_sprite = nullptr;
	current_enemy = nullptr;
	current_cutscene = nullptr;
}

void editor::draw() {
	camera::set_camera("Editor");

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
			ImGui::Text("Game Stats");

			ImGui::Separator();
			ImGui::Text("Screen Size: (%i, %i)", renderer::screen_width, renderer::screen_height);
			
			ImGui::Separator();
			ImGui::Text("Number of:");
			ImGui::Text(" - Sprites %i", current_level.sprites.size());
			ImGui::Text(" - Path Nodes %i", current_level.path_nodes.size());

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
		window_flags |= ImGuiWindowFlags_MenuBar;

		ImGui::SetNextWindowPos(ImVec2(work_size.x, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(300.0f, work_size.y), ImGuiCond_Always);

		if (ImGui::Begin("LEVEL DATA", &p_open, window_flags)) {
			ImGui::InputText("Level Name", &current_level.name);

			if (ImGui::Button("Save")) {
				if (!current_level.name.empty()) {
					current_level.save();
				}
				else {
					printf("Name Empty\n");
				}
			}
			
			ImGui::SameLine();

			if (ImGui::Button("Load")) {
				if (!current_level.name.empty()) {
					clear_selected();
					current_level.load(current_level.name);
				}
				else {
					printf("Name Empty\n");
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Clear")) {
				clear_selected();
				current_level.clean();
			}

			ImGui::Separator();

			static const char* entity_types[] = { "path", "sprites", "cutscene" };
			static const char* current_entity_type = entity_types[0];

			if(ImGui::BeginCombo("Types", current_entity_type)) {
				for (size_t i = 0; i < IM_ARRAYSIZE(entity_types); i++) {
					bool is_selected = (current_entity_type == entity_types[i]);
					if (ImGui::Selectable(entity_types[i], is_selected))
						current_entity_type = entity_types[i];
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (current_entity_type != NULL) {
				/* PATH */if (current_entity_type == entity_types[0]) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& node : current_level.path_nodes) {
							const bool is_selected = (current_path_node != nullptr) && (current_path_node == node);

							std::string label = "Node";

							ImGui::PushID(&node);
							if (ImGui::Selectable(label.c_str(), is_selected)) {
								current_obstacle = nullptr;
								current_enemy = nullptr;

								current_path_node = node;
							}

							if (is_selected) {
								ImGui::SetItemDefaultFocus();
							}

							ImGui::PopID();
						}

						ImGui::ListBoxFooter();
					}

					if (ImGui::Button("+")) {
						auto n = std::make_shared<path_node>();

						if (!current_level.path_nodes.empty())
							n->pos = current_level.path_nodes.back()->pos;

						current_level.path_nodes.push_back(n);
						current_path_node = current_level.path_nodes.back();
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						current_level.path_nodes.erase(
							std::remove(current_level.path_nodes.begin(),
							current_level.path_nodes.end(), current_path_node),
							current_level.path_nodes.end());

						if (!current_level.path_nodes.empty())
							current_path_node = current_level.path_nodes.back();
						else
							current_path_node = nullptr;
					}

					ImGui::Separator();

					if (current_path_node != nullptr) {
						ImGui::PushID(&current_path_node);
						ImGui::DragFloat2("Position", (float*)&current_path_node->pos);

						ImGui::Spacing();
						ImGui::Text("Flags");

						const float button_width = ImGui::GetWindowSize().x * 0.25f;

						ImGui::InputText("Trigger Event Name", &current_path_node->trigger_event_name, ImGuiInputTextFlags_CharsNoBlank);

						/* SLOW */ {
							if (ImGui::Button("Slow", { button_width, 0.0f }))
								PATH_NODE_FLAG_TOGGLE(&current_path_node->flags, PATH_NODE_SLOW);

							ImGui::SameLine();
							ImGui::Text((current_path_node->flags & PATH_NODE_SLOW) ? "ON" : "OFF");
						}
						/* FAST */ {
							if (ImGui::Button("Fast", { button_width, 0.0f }))
								PATH_NODE_FLAG_TOGGLE(&current_path_node->flags, PATH_NODE_FAST);

							ImGui::SameLine();
							ImGui::Text((current_path_node->flags & PATH_NODE_FAST) ? "ON" : "OFF");
						}

						ImGui::Separator();

						if (ImGui::CollapsingHeader("Obstacles")) {
							ImGui::PushID(&current_path_node->obstacles);
							if (ImGui::ListBoxHeader("", { -1,0 })) {
								for (auto& obstacle : current_path_node->obstacles) {
									const bool is_selected = (current_obstacle != nullptr) && (current_obstacle == obstacle);

									std::string label = "Obstacle";

									ImGui::PushID(&obstacle);
									if (ImGui::Selectable(label.c_str(), is_selected)) {
										current_obstacle = obstacle;
									}

									if (is_selected) {
										ImGui::SetItemDefaultFocus();
									}

									ImGui::PopID();
								}
								ImGui::ListBoxFooter();
							}

							if (ImGui::Button("+")) {
								auto o = std::make_shared<obstacle_entity>();
								o->pos = current_path_node->pos;
								current_path_node->obstacles.push_back(o);
								current_obstacle = o;
							}

							ImGui::SameLine();
							if (ImGui::Button("-")) {
								current_path_node->obstacles.erase(
									std::remove(current_path_node->obstacles.begin(),
									current_path_node->obstacles.end(), current_obstacle),
									current_path_node->obstacles.end());

								if (!current_path_node->obstacles.empty())
									current_obstacle = current_path_node->obstacles.back();
								else
									current_obstacle = nullptr;
							}

							if (current_obstacle != nullptr) {
								ImGui::PushID(current_obstacle.get());

								ImGui::DragFloat2("Position", (float*)&current_obstacle->pos);

								ImGui::PopID();
							}
							ImGui::PopID();
						}
						if (ImGui::CollapsingHeader("Enemies")) {
							ImGui::PushID(&current_path_node->enemies);
							if (ImGui::ListBoxHeader("", { -1,0 })) {
								for (auto& enemy : current_path_node->enemies) {
									const bool is_selected = (current_enemy != nullptr) && (current_enemy == enemy);

									std::string label = "Enemy";

									ImGui::PushID(&enemy);
									if (ImGui::Selectable(label.c_str(), is_selected)) {
										current_enemy = enemy;
									}

									if (is_selected) {
										ImGui::SetItemDefaultFocus();
									}

									ImGui::PopID();
								}
								ImGui::ListBoxFooter();
							}

							if (ImGui::Button("+")) {
								auto e = std::make_shared<enemy_entity>();
								e->pos = current_path_node->pos;
								current_path_node->enemies.push_back(e);
								current_enemy = e;
							}

							ImGui::SameLine();
							if (ImGui::Button("-")) {
								current_path_node->enemies.erase(
									std::remove(current_path_node->enemies.begin(),
									current_path_node->enemies.end(), current_enemy),
									current_path_node->enemies.end());

								if (!current_path_node->enemies.empty())
									current_enemy = current_path_node->enemies.back();
								else
									current_enemy = nullptr;
							}

							if (current_enemy != nullptr) {
								ImGui::PushID(current_enemy.get());

								ImGui::DragFloat2("Position", (float*)&current_enemy->pos);

								ImGui::PopID();
							}
							ImGui::PopID();
						}

						ImGui::PopID();
					}
				}
				/* SPRITES */if (current_entity_type == entity_types[1]) {
					// TODO: finally implement sprite creation
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& sprite : current_level.sprites) {
							const bool is_selected = (current_sprite != nullptr) && (current_sprite == sprite);

							std::string label = "Sprite";

							ImGui::PushID(&sprite);
							if (ImGui::Selectable(label.c_str(), is_selected)) {
								current_sprite = sprite;
							}

							if (is_selected) {
								ImGui::SetItemDefaultFocus();
							}

							ImGui::PopID();
						}

						ImGui::ListBoxFooter();
					}

					if (ImGui::Button("+")) {
						auto s = std::make_shared<sprite_entity>();

						current_level.sprites.push_back(s);
						current_sprite = current_level.sprites.back();
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						current_level.sprites.erase(
							std::remove(current_level.sprites.begin(),
							current_level.sprites.end(), current_sprite),
							current_level.sprites.end());

						if (!current_level.sprites.empty())
							current_sprite = current_level.sprites.back();
						else
							current_sprite = nullptr;
					}

					ImGui::Separator();

					if (current_sprite != nullptr) {
						ImGui::PushID(&current_sprite->pos);
						ImGui::DragFloat2("Position", (float*)&current_sprite->pos);
						ImGui::PopID();

						ImGui::PushID(&current_sprite->spr->size);
						ImGui::DragInt2("Size", (int*)&current_sprite->spr->size);
						ImGui::PopID();

						ImGui::PushID(&current_sprite->spr->layer);
						ImGui::DragInt("Layer", &current_sprite->spr->layer);
						ImGui::PopID();

						ImGui::PushID(&current_sprite->spr->colour);
						float color[3] = { current_sprite->spr->colour.x,
										   current_sprite->spr->colour.y,
										   current_sprite->spr->colour.z };
						ImGui::ColorEdit3("Colour", color);
						current_sprite->spr->colour = { color[0], color[1], color[2] };
						ImGui::PopID();

						/*
						std::string name;

						ImGui::InputText("Path", &name, ImGuiInputTextFlags_EnterReturnsTrue);

						if (ImGui::Button("Set Path")) {
							current_sprite->sprite_path = name;
							current_sprite->spr->set_sprite_path(name.c_str());
						}
						*/
					}
				}
				/* CUTSCENES */if (current_entity_type == entity_types[2]) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& cutscene : current_level.cutscenes) {
							const bool is_selected = (current_cutscene != nullptr) && (current_cutscene == cutscene);

							std::string label = "Cutscene";

							ImGui::PushID(&cutscene);
							if (ImGui::Selectable(label.c_str(), is_selected)) {
								current_cutscene = cutscene;
							}

							if (is_selected) {
								ImGui::SetItemDefaultFocus();
							}

							ImGui::PopID();
						}

						ImGui::ListBoxFooter();
					}

					if (ImGui::Button("+")) {
						auto c = std::make_shared<cutscene>();

						current_level.cutscenes.push_back(c);
						current_cutscene = current_level.cutscenes.back();
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						current_level.cutscenes.erase(
							std::remove(current_level.cutscenes.begin(),
							current_level.cutscenes.end(), current_cutscene),
							current_level.cutscenes.end());

						if (!current_level.cutscenes.empty())
							current_cutscene = current_level.cutscenes.back();
						else
							current_cutscene = nullptr;
					}

					ImGui::Separator();

					if (current_cutscene != nullptr) {
						ImGui::PushID(&current_cutscene);

						ImGui::InputText("event name", &current_cutscene->event_name, ImGuiInputTextFlags_CharsNoBlank);

						ImGui::PopID();
					}
				}
			}

			ImGui::End();
		}
	}

	if (current_path_node != nullptr) {
		float box_size = 10.0f;
		renderer::debug::draw_box({ current_path_node->pos.x - (box_size / 2), current_path_node->pos.y - (box_size / 2) }, { box_size, box_size }, colour::black);
		renderer::debug::draw_box_wireframe({ current_path_node->pos.x - (box_size / 2), current_path_node->pos.y - (box_size / 2) }, { box_size, box_size }, colour::white);
	}

	path_node* previous_node = nullptr;
	for (auto& node : current_level.path_nodes) {
		glm::vec3 colour = colour::cyan;

		if (previous_node != nullptr)
			renderer::debug::draw_line(node->pos, previous_node->pos, colour::green);

		renderer::debug::draw_circle(node->pos, 2, colour);
		previous_node = node.get();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void editor::clean() {
	clear_selected();

	current_level.clean();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	printf("-----------------\n");
	printf("EDITOR CLEANED UP\n");
}

#endif // _DEBUG
