#ifdef _DEBUG

#include "editor.h"

#include <SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include "input.h"
#include "renderer.h"

#include "scene.h"

#include "entity.h"
#include "player.h"
#include "obstacle.h"

void editor::init() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(renderer::window, renderer::context);
	ImGui_ImplOpenGL3_Init("#version 330");
}

Uint64 start, end;
float FPS;

glm::vec2 velocity;
float speed = 2;

void editor::update(double dt) {
	start = SDL_GetPerformanceCounter();

	/*
	if (input::button_pressed("MoveUp") && input::button_pressed("MoveDown")) { velocity.y = 0; }
	else if (input::button_pressed("MoveUp")) { velocity.y = 1; }
	else if (input::button_pressed("MoveDown")) { velocity.y = -1; }
	else { velocity.y = 0; }

	if (input::button_pressed("MoveLeft") && input::button_pressed("MoveRight")) { velocity.x = 0; }
	else if (input::button_pressed("MoveLeft")) { velocity.x = -1; }
	else if (input::button_pressed("MoveRight")) { velocity.x = 1; }
	else { velocity.x = 0; }

	glm::vec2 moveVector = glm::vec2(std::floor(velocity.x), std::floor(velocity.y));
	moveVector /= speed;
	moveVector *= dt;
	*/

	if (input::mouse_button_pressed(input::MOUSE_RIGHT)) {
		glm::vec2 mouseDelta = { -input::get_mouse_delta().x, input::get_mouse_delta().y };

		renderer::view = glm::translate(renderer::view,
			{ mouseDelta, 0.0f });
	}

	/*
	float zoom = 1;
	if (input::button_down("Shoot")) {
		zoom += .1f;
	}
	else if (input::button_down("Aim")) {
		zoom -= .1f;
	}

	renderer::view = glm::scale(renderer::view, { zoom, zoom, 0 });
	*/

	/*
	if (input::button_down("Shoot")) {
		renderer::view = glm::scale(renderer::view, { 0.9f, 0.9f, 0 });
	}
	else if (input::button_down("Aim")) {
		renderer::view = glm::scale(renderer::view, { 1.1f, 1.1f, 0 });
	}
	*/
}

static std::shared_ptr<obstacle> current_obstacle = nullptr;
static glm::vec2* current_path_node = nullptr;
static renderer::sprite* current_sprite = nullptr;

void editor::draw() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(renderer::window);
	ImGui::NewFrame();

	const float PAD = 10.0f;
	static bool p_open = false;

	ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(PAD, PAD));
	/* ----- OVERLAY ----- */ {
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0.0f, 0.0f));

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
		window_flags |= ImGuiWindowFlags_NoNav;

		ImGui::SetNextWindowBgAlpha(0.9f);
		if (ImGui::Begin("OVERLAY", &p_open, window_flags)) {
			ImGui::Text("Game Stats");

			ImGui::Separator();
			ImGui::Text("Screen Size: (%i, %i)", renderer::screen_width, renderer::screen_height);
			ImGui::Text("%f FPS", FPS);

			ImGui::End();
		}
	}

	static bool save_menu_open = false;
	static bool load_menu_open = false;
	/* ----- SAVE / LOAD MENU ----- */ {
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

		ImGuiInputTextFlags input_text_flags = 0;
		input_text_flags |= ImGuiInputTextFlags_EnterReturnsTrue;

		if (save_menu_open) {
			if (ImGui::Begin("Save", &p_open, window_flags)) {
				static char name[128];

				if (ImGui::InputText("", name, IM_ARRAYSIZE(name), input_text_flags)) {
					level::save(name);
					save_menu_open = false;
				}


				ImGui::SameLine();
				if (ImGui::Button("SAVE")) {
					level::save(name);
					save_menu_open = false;
				}

				ImGui::SameLine();
				if (ImGui::Button("CLOSE")) { save_menu_open = false; }

				ImGui::End();
			}
		}

		if (load_menu_open) {
			if (ImGui::Begin("Load", &p_open, window_flags)) {
				static char name[128];
				if (ImGui::InputText("", name, IM_ARRAYSIZE(name), input_text_flags)) {
					level::load(name);
					load_menu_open = false;
				}

				ImGui::SameLine();
				if (ImGui::Button("LOAD")) {
					level::load(name);
					load_menu_open = false;
				}

				ImGui::SameLine();
				if (ImGui::Button("CLOSE")) { load_menu_open = false; }

				ImGui::End();
			}
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

		// Entites List Window
		ImGui::SetNextWindowPos(ImVec2(work_size.x, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(300.0f, work_size.y), ImGuiCond_Always);

		ImGui::SetNextWindowBgAlpha(0.9f);

		if (ImGui::Begin("LEVEL DATA", &p_open, window_flags)) {
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("New")) {
						level::clean(); current_obstacle = nullptr;
					}

					if (ImGui::MenuItem("Save")) {
						save_menu_open = true;
					}

					if (ImGui::MenuItem("Load")) {
						load_menu_open = true;
						current_obstacle = nullptr;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			if (ImGui::BeginTabBar("TABS")) {
				if (ImGui::BeginTabItem("PATH")) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& node : level::data.path_nodes) {
							const bool is_selected = (current_path_node != nullptr) && (current_path_node == &node);

							std::string label = "Node";

							ImGui::PushID(&node);
							if (ImGui::Selectable(label.c_str(), is_selected)) {
								current_path_node = &node;
							}

							if (is_selected) {
								ImGui::SetItemDefaultFocus();
							}

							ImGui::PopID();
						}

						ImGui::ListBoxFooter();
					}

					if (ImGui::Button("+")) {
						glm::vec2 n = { 0,0 };
						level::data.path_nodes.push_back(n);
						current_path_node = &level::data.path_nodes.back();
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						level::data.path_nodes.erase(
							std::remove(level::data.path_nodes.begin(),
							level::data.path_nodes.end(), *current_path_node),
							level::data.path_nodes.end());

						if (!level::data.path_nodes.empty())
							current_path_node = &level::data.path_nodes.back();
						else
							current_path_node = nullptr;
					}

					ImGui::Separator();

					if (current_path_node != nullptr) {
						ImGui::PushID(current_path_node);

						ImGui::DragFloat("Position.x", &current_path_node->x);
						ImGui::DragFloat("Position.y", &current_path_node->y);

						ImGui::PopID();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("OBSTACLES")) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto const& entity : level::data.obstacles) {
							const bool is_selected = (current_obstacle != nullptr) && (current_obstacle->id == entity->id);

							std::string label = "Obstacle" + std::to_string(entity->id);

							ImGui::PushID(entity->id);
							if (ImGui::Selectable(label.c_str(), is_selected)) {
								current_obstacle = entity;
							}

							if (is_selected) {
								ImGui::SetItemDefaultFocus();
							}

							ImGui::PopID();
						}

						ImGui::ListBoxFooter();
					}

					if (ImGui::Button("+")) {
						auto o = std::make_shared<obstacle>();
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
						ImGui::PushID(current_obstacle->id);

						ImGui::DragFloat("Position.x", &current_obstacle->pos.x);
						ImGui::DragFloat("Position.y", &current_obstacle->pos.y);

						ImGui::PopID();
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("ENEMIES")) {
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("SPRITES")) {
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
						glm::vec2 pos = { 0,0 };
						renderer::sprite* s = renderer::create_sprite("data/textures/face.png", &pos, 0);
						level::data.sprites.push_back(s);
						current_sprite = level::data.sprites.back();
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						
						level::data.sprites.erase(
							std::remove(level::data.sprites.begin(),
							level::data.sprites.end(), current_sprite),
							level::data.sprites.end());

						renderer::delete_sprite(current_sprite);
						if (!level::data.sprites.empty())
							current_sprite = level::data.sprites.back();
						else
							current_sprite = nullptr;
					}

					ImGui::Separator();

					if (current_path_node != nullptr) {
						ImGui::PushID(current_sprite);

						ImGui::PopID();
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			ImGui::End();
		}
	}
	ImGui::PopStyleVar();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	end = SDL_GetPerformanceCounter();

	float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency();
	FPS = 1.0f / elapsed;
}

void editor::clean() {
	current_obstacle = nullptr;
	current_path_node = nullptr;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	printf("-----------------\n");
	printf("EDITOR CLEANED UP\n");
}

#endif // _DEBUG