#include "editor.h"

#ifdef _DEBUG
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <string>

#include "input.h"
#include "renderer.h"
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

entity_manager* manager;

void editor::init() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(renderer::window, renderer::context);
	ImGui_ImplOpenGL3_Init("#version 330");

	manager->init();

	camera::register_camera("Editor", std::make_shared<editor_camera>());
}

glm::vec2 velocity;
float speed = 2;

static std::shared_ptr<entity> current_entity = nullptr;

void editor::clear_selected() {
	current_entity = nullptr;
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
			ImGui::Text(" - Entities %i", manager->entities.size());

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
			ImGui::InputText("Level Name", &manager->name);

			if (ImGui::Button("Save")) {
				if (!manager->name.empty()) {
					manager->save();
				}
				else {
					printf("Name Empty\n");
				}
			}
			
			ImGui::SameLine();

			if (ImGui::Button("Load")) {
				if (!manager->name.empty()) {
					clear_selected();
					manager->load(manager->name);
				}
				else {
					printf("Name Empty\n");
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Clear")) {
				clear_selected();
				manager->clean();
			}

			ImGui::Separator();

			ImGui::End();
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void editor::clean() {
	clear_selected();

	manager->clean();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	printf("-----------------\n");
	printf("EDITOR CLEANED UP\n");
}

#endif // _DEBUG
