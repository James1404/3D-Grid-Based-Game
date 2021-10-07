#include "editor.h"

#ifdef _DEBUG
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include "input.h"
#include "renderer.h"
#include "scene.h"
#include "entity.h"
#include "player.h"

level editor::editor_level;

void editor::init() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(renderer::window, renderer::context);
	ImGui_ImplOpenGL3_Init("#version 330");

	editor_level.init();
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

static std::shared_ptr<obstacle_entity> current_obstacle = nullptr;
static glm::vec2* current_path_node = nullptr;
static std::shared_ptr<sprite_entity> current_sprite = nullptr;
static std::shared_ptr<enemy_entity> current_enemy = nullptr;

void clear_selected() {
	current_obstacle = nullptr;
	current_path_node = nullptr;
	current_sprite = nullptr;
	current_enemy = nullptr;
}

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
					editor_level.save();
					save_menu_open = false;
				}

				ImGui::SameLine();

				if (ImGui::Button("SAVE")) {
					editor_level.save();
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
					clear_selected();

					editor_level.load(name);
					load_menu_open = false;
				}

				ImGui::SameLine();
				if (ImGui::Button("LOAD")) {
					clear_selected();

					editor_level.load(name);
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
						clear_selected();
						editor_level.clean();
					}

					if (ImGui::MenuItem("Save")) {
						save_menu_open = true;
					}

					if (ImGui::MenuItem("Load")) {
						load_menu_open = true;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			static char name[256];
			ImGui::InputText("Level Name", name, IM_ARRAYSIZE(name));
			editor_level.name = name;

			if (ImGui::Button("Save")) {
				if (strlen(name) != 0) {
					editor_level.save();
				}
				else {
					printf("Name Empty\n");
				}
			}

			ImGui::Separator();

			if (ImGui::BeginTabBar("TABS")) {
				if (ImGui::BeginTabItem("PATH")) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& node : editor_level.path_nodes) {
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
						editor_level.path_nodes.push_back(n);
						current_path_node = &editor_level.path_nodes.back();
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						editor_level.path_nodes.erase(
							std::remove(editor_level.path_nodes.begin(),
							editor_level.path_nodes.end(), *current_path_node),
							editor_level.path_nodes.end());

						if (!editor_level.path_nodes.empty())
							current_path_node = &editor_level.path_nodes.back();
						else
							current_path_node = nullptr;
					}

					ImGui::Separator();

					if (current_path_node != nullptr) {
						ImGui::PushID(current_path_node);
						ImGui::Text("Position");

						ImGui::DragFloat("X", &current_path_node->x);
						ImGui::DragFloat("Y", &current_path_node->y);

						ImGui::PopID();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("OBSTACLES")) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& obstacle : editor_level.obstacles) {
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
						editor_level.obstacles.push_back(o);
						current_obstacle = o;
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						editor_level.obstacles.erase(
							std::remove(editor_level.obstacles.begin(),
							editor_level.obstacles.end(), current_obstacle),
							editor_level.obstacles.end());

						if (!editor_level.obstacles.empty())
							current_obstacle = editor_level.obstacles.back();
						else
							current_obstacle = nullptr;
					}

					ImGui::Separator();

					if (current_obstacle != nullptr) {
						ImGui::PushID(current_obstacle.get());
						ImGui::Text("Position");

						ImGui::DragFloat("X", &current_obstacle->pos.x);
						ImGui::DragFloat("Y", &current_obstacle->pos.y);

						ImGui::PopID();
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("ENEMIES")) {
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& enemy : editor_level.enemies) {
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
						editor_level.enemies.push_back(e);
						current_enemy = e;
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						editor_level.enemies.erase(
							std::remove(editor_level.enemies.begin(),
							editor_level.enemies.end(), current_enemy),
							editor_level.enemies.end());

						if (!editor_level.enemies.empty())
							current_enemy = editor_level.enemies.back();
						else
							current_enemy = nullptr;
					}

					ImGui::Separator();

					if (current_enemy != nullptr) {
						ImGui::PushID(current_enemy.get());
						ImGui::Text("Position");

						ImGui::DragFloat("X", &current_enemy->pos.x);
						ImGui::DragFloat("Y", &current_enemy->pos.y);

						ImGui::PopID();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("SPRITES")) {
					// TODO: finally implement sprite creation
					if (ImGui::ListBoxHeader("", { -1,0 })) {
						for (auto& sprite : editor_level.sprites) {
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

						editor_level.sprites.push_back(s);
						current_sprite = editor_level.sprites.back();
					}

					ImGui::SameLine();
					if (ImGui::Button("-")) {
						editor_level.sprites.erase(
							std::remove(editor_level.sprites.begin(),
							editor_level.sprites.end(), current_sprite),
							editor_level.sprites.end());

						if (!editor_level.sprites.empty())
							current_sprite = editor_level.sprites.back();
						else
							current_sprite = nullptr;
					}

					ImGui::Separator();

					if (current_sprite != nullptr) {
						ImGui::PushID(current_sprite.get());

						ImGui::Text("Position");

						ImGui::DragFloat("X", &current_sprite->pos.x);
						ImGui::DragFloat("Y", &current_sprite->pos.y);

						ImGui::DragInt("Layer", &current_sprite->spr->layer);

						static char name[128];

						ImGui::InputText("Path", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_EnterReturnsTrue);

						if (ImGui::Button("Set Path")) {
							current_sprite->sprite_path = name;
							current_sprite->spr->set_sprite_path(name);
						}

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
	clear_selected();

	editor_level.clean();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	printf("-----------------\n");
	printf("EDITOR CLEANED UP\n");
}

#endif // _DEBUG