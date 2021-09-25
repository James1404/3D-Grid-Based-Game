#ifdef _DEBUG

#include "editor.h"

#include <SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

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

	if (input::button_pressed("MoveUp")) { velocity.y = 1; }
	else if (input::button_pressed("MoveDown")) { velocity.y = -1; }
	else { velocity.y = 0; }

	if (input::button_pressed("MoveLeft")) { velocity.x = -1; }
	else if (input::button_pressed("MoveRight")) { velocity.x = 1; }
	else { velocity.x = 0; }

	glm::vec2 moveVector = glm::vec2(std::floor(velocity.x), std::floor(velocity.y));
	moveVector /= speed;
	moveVector *= dt;

	renderer::view = glm::translate(renderer::view, glm::vec3(-moveVector, 0.0f));
}

// static std::shared_ptr<entity> selectedEntity;

void editor::draw() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(renderer::window);
	ImGui::NewFrame();

	const float PAD = 10.0f;
	static bool p_open = true;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(PAD, PAD));
	/* ----- OVERLAY ----- */ {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_size = viewport->WorkSize;

		ImGui::SetNextWindowPos(ImVec2(PAD, PAD), ImGuiCond_Always, ImVec2(0.0f, 0.0f));

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
		window_flags |= ImGuiWindowFlags_NoNav;

		ImGui::SetNextWindowBgAlpha(0.9f);
		if (ImGui::Begin("Overlay", &p_open, window_flags)) {
			ImGui::Text("Game Stats");

			ImGui::Separator();
			ImGui::Text("Screen Size: (%i, %i)", renderer::screen_width, renderer::screen_height);
			ImGui::Text("%f FPS", FPS);

			ImGui::End();
		}
	}
	/* ----- INSPECTOR ----- */ {
		// Set Windows Flags
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
		window_flags |= ImGuiWindowFlags_MenuBar;

		// Entites List Window
		ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;
		ImGui::SetNextWindowPos(ImVec2(work_size.x - PAD, PAD), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(300.0f, (work_size.y / 2) - (PAD * 1.5f)), ImGuiCond_Always);

		ImGui::SetNextWindowBgAlpha(0.9f);

		if (ImGui::Begin("Entities", &p_open, window_flags)) {
			if (ImGui::BeginMenuBar()) {
				// Custom save / load name
				if (ImGui::BeginMenu("Map")) {
					if (ImGui::MenuItem("Save")) { level::save(); }
					if (ImGui::MenuItem("Load")) { level::load(); }
					ImGui::EndMenu();
				}

				/*
				glm::vec2 spawnPos = { 0,0 };
				if (ImGui::BeginMenu("Create")) {
					if (ImGui::MenuItem("Create Obstacle")) {
						auto o = std::make_unique<obstacle>();
						o->pos = spawnPos;
						level::data.obstacles.push_back(std::move(o));
						selectedEntity = level::data.obstacles.back();
					}
					ImGui::EndMenu();
				}
				*/
				ImGui::EndMenuBar();
			}

			/*
			if (ImGui::ListBoxHeader("Entities")) {
				for (auto const& entity : level::current_level.obstacles) {
					const bool is_selected = (selectedEntity != nullptr) && (selectedEntity->id == entity->id);

					ImGui::PushID(entity->id);
					if (ImGui::Selectable(entity->name, is_selected)) {
						selectedEntity = entity;
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
					ImGui::PopID();
				}

				ImGui::ListBoxFooter();
			}
			*/
			ImGui::End();
		}

		ImGui::PopStyleVar();
	}
	/* ----- TOOLBAR ----- */ {
		// Set Windows Flags
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;

		// Toolbar Window
		ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;
		ImGui::SetNextWindowPos(ImVec2(work_size.x - PAD, work_size.y - PAD), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
		ImGui::SetNextWindowSize(ImVec2(300.0f, (work_size.y / 2) - (PAD * 1.5f)), ImGuiCond_Always);

		ImGui::SetNextWindowBgAlpha(0.9f);

		if (ImGui::Begin("Toolbar", &p_open, window_flags)) {
			/*
			if(selectedEntity != nullptr)
				selectedEntity->editor_draw();
			*/
			ImGui::End();
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	end = SDL_GetPerformanceCounter();

	float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency();
	FPS = 1.0f / elapsed;
}

void editor::clean() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

#endif // _DEBUG