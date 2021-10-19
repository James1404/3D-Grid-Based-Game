#include "editor.h"

#ifdef _DEBUG
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

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
			//	which fixed panning speed.
			glm::vec2 mouseDelta = { -input::get_mouse_delta().x, input::get_mouse_delta().y };
			view = glm::translate(view, { mouseDelta, 0.0f });
		}

		// TODO: it zooms into the center of the game, not the center of the camera
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

	level::init();

	camera::register_camera("Editor", std::make_shared<editor_camera>());
}

glm::vec2 velocity;
float speed = 2;

static std::shared_ptr<obstacle_entity> current_obstacle = nullptr;
static std::shared_ptr<path_node> current_path_node = nullptr;
static std::shared_ptr<sprite_entity> current_sprite = nullptr;
static std::shared_ptr<enemy_entity> current_enemy = nullptr;
static std::shared_ptr<trigger_entity> current_trigger = nullptr;

void editor::clear_selected() {
	current_obstacle = nullptr;
	current_path_node = nullptr;
	current_sprite = nullptr;
	current_enemy = nullptr;
	current_trigger = nullptr;
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
			ImGui::Text(" - Obstacles %i", level::data.obstacles.size());
			ImGui::Text(" - Sprites %i", level::data.sprites.size());
			ImGui::Text(" - Enemies %i", level::data.enemies.size());
			ImGui::Text(" - Path Nodes %i", level::data.path_nodes.size());

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
			ImGui::InputText("Level Name", &level::data.name);

			if (ImGui::Button("Save")) {
				if (!level::data.name.empty()) {
					level::save();
				}
				else {
					printf("Name Empty\n");
				}
			}
			
			ImGui::SameLine();

			if (ImGui::Button("Load")) {
				if (!level::data.name.empty()) {
					clear_selected();
					level::load(level::data.name);
				}
				else {
					printf("Name Empty\n");
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Clear")) {
				clear_selected();
				level::clean();
			}

			ImGui::Separator();

			if (ImGui::BeginTabBar("TABS")) {
				if (ImGui::BeginTabItem("PATH")) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& node : level::data.path_nodes) {
							const bool is_selected = (current_path_node != nullptr) && (current_path_node == node);

							std::string label = "Node";

							ImGui::PushID(&node);
							if (ImGui::Selectable(label.c_str(), is_selected)) {
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

						if (!level::data.path_nodes.empty())
							n->pos = level::data.path_nodes.back()->pos;

						level::data.path_nodes.push_back(n);
						current_path_node = level::data.path_nodes.back();
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						level::data.path_nodes.erase(
							std::remove(level::data.path_nodes.begin(),
							level::data.path_nodes.end(), current_path_node),
							level::data.path_nodes.end());

						if (!level::data.path_nodes.empty())
							current_path_node = level::data.path_nodes.back();
						else
							current_path_node = nullptr;
					}

					ImGui::Separator();

					if (current_path_node != nullptr) {
						ImGui::PushID(&current_path_node);
						ImGui::DragFloat2("Position", (float*)&current_path_node->pos);

						ImGui::Spacing();
						ImGui::Text("Flags");
						
						static const float button_width = 0.25f;

						/* COMBAT */ {
							if (ImGui::Button("Combat", { ImGui::GetWindowSize().x * button_width, 0.0f }))
								PATH_NODE_FLAG_TOGGLE(&current_path_node->flags, PATH_NODE_COMBAT);

							ImGui::SameLine();
							ImGui::Text((current_path_node->flags & PATH_NODE_COMBAT) ? "ON" : "OFF");
						}
						/* SLOW */ {
							if (ImGui::Button("Slow", { ImGui::GetWindowSize().x * button_width, 0.0f }))
								PATH_NODE_FLAG_TOGGLE(&current_path_node->flags, PATH_NODE_SLOW);

							ImGui::SameLine();
							ImGui::Text((current_path_node->flags & PATH_NODE_SLOW) ? "ON" : "OFF");
						}
						/* FAST */ {
							if (ImGui::Button("Fast", { ImGui::GetWindowSize().x * button_width, 0.0f }))
								PATH_NODE_FLAG_TOGGLE(&current_path_node->flags, PATH_NODE_FAST);

							ImGui::SameLine();
							ImGui::Text((current_path_node->flags & PATH_NODE_FAST) ? "ON" : "OFF");
						}

						ImGui::PopID();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("OBSTACLES")) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& obstacle : level::data.obstacles) {
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
						o->pos = { 0,0 };
						level::data.obstacles.push_back(o);
						current_obstacle = o;
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						level::data.obstacles.erase(
							std::remove(level::data.obstacles.begin(),
							level::data.obstacles.end(), current_obstacle),
							level::data.obstacles.end());

						if (!level::data.obstacles.empty())
							current_obstacle = level::data.obstacles.back();
						else
							current_obstacle = nullptr;
					}

					ImGui::Separator();

					if (current_obstacle != nullptr) {
						ImGui::PushID(current_obstacle.get());

						ImGui::DragFloat2("Position", (float*)&current_obstacle->pos);

						ImGui::PopID();
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("ENEMIES")) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& enemy : level::data.enemies) {
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
						e->pos = { 0,0 };
						level::data.enemies.push_back(e);
						current_enemy = e;
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						level::data.enemies.erase(
							std::remove(level::data.enemies.begin(),
							level::data.enemies.end(), current_enemy),
							level::data.enemies.end());

						if (!level::data.enemies.empty())
							current_enemy = level::data.enemies.back();
						else
							current_enemy = nullptr;
					}

					ImGui::Separator();

					if (current_enemy != nullptr) {
						ImGui::PushID(current_enemy.get());

						ImGui::DragFloat2("Position", (float*)&current_enemy->pos);

						ImGui::PopID();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("SPRITES")) {
					// TODO: finally implement sprite creation
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& sprite : level::data.sprites) {
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

						level::data.sprites.push_back(s);
						current_sprite = level::data.sprites.back();
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						level::data.sprites.erase(
							std::remove(level::data.sprites.begin(),
							level::data.sprites.end(), current_sprite),
							level::data.sprites.end());

						if (!level::data.sprites.empty())
							current_sprite = level::data.sprites.back();
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
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("TRIGGER")) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& node : level::data.triggers) {
							const bool is_selected = (current_trigger != nullptr) && (current_trigger == node);

							std::string label = "Trigger";

							ImGui::PushID(&node);
							if (ImGui::Selectable(label.c_str(), is_selected)) {
								current_trigger = node;
							}

							if (is_selected) {
								ImGui::SetItemDefaultFocus();
							}

							ImGui::PopID();
						}

						ImGui::ListBoxFooter();
					}

					if (ImGui::Button("+")) {
						auto t = std::make_shared<trigger_entity>();

						if (!level::data.triggers.empty())
							t->pos = level::data.triggers.back()->pos;

						level::data.triggers.push_back(t);
						current_trigger = level::data.triggers.back();
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						level::data.triggers.erase(
							std::remove(level::data.triggers.begin(),
							level::data.triggers.end(), current_trigger),
							level::data.triggers.end());

						if (!level::data.triggers.empty())
							current_trigger = level::data.triggers.back();
						else
							current_trigger = nullptr;
					}

					ImGui::Separator();

					if (current_trigger != nullptr) {
						ImGui::PushID(&current_trigger);
						ImGui::DragFloat2("Position", (float*)&current_trigger->pos);
						ImGui::DragInt2("Size", (int*)&current_trigger->size);

						ImGui::PopID();
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			ImGui::End();
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void editor::clean() {
	clear_selected();

	level::clean();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	printf("-----------------\n");
	printf("EDITOR CLEANED UP\n");
}

#endif // _DEBUG
