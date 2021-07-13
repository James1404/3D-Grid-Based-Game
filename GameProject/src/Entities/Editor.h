#pragma once
#include "../Entity.h"
#include "../Game.h"

class Editor : public Entity {
public:
	void init() override {
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplSDL2_InitForOpenGL(Game::window, Game::context);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	void update(double dt) override {
		if (Game::event.type == SDL_KEYDOWN) {
			switch (Game::event.key.keysym.sym) {
			case SDLK_w:
				this->velocity.y = -1;
				break;
			case SDLK_s:
				this->velocity.y = 1;
				break;
			case SDLK_a:
				this->velocity.x = 1;
				break;
			case SDLK_d:
				this->velocity.x = -1;
				break;
			default:
				break;
			}
		}

		if (Game::event.type == SDL_KEYUP) {
			switch (Game::event.key.keysym.sym) {
			case SDLK_w:
				this->velocity.y = 0;
				break;
			case SDLK_s:
				this->velocity.y = 0;
				break;
			case SDLK_a:
				this->velocity.x = 0;
				break;
			case SDLK_d:
				this->velocity.x = 0;
				break;
			default:
				break;
			}
		}

		glm::vec2 moveVector = glm::vec2(std::floor(velocity.x), std::floor(velocity.y));
		moveVector /= speed;
		moveVector *= dt;

		Game::view = glm::translate(Game::view, glm::vec3(moveVector, 0.0f));
	}

	void render() override {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(Game::window);
		ImGui::NewFrame();

		const float PAD = 10.0f;
		static bool p_open = true;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(PAD, PAD));

		{
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
				ImGui::Text("Screen Size: (%i, %i)", Game::screen_width, Game::screen_height);
				ImGui::Text("No. of Entities: %i", Game::scene.entities.size());

				ImGui::End();
			}
		}

		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 work_size = viewport->WorkSize;

			// Set Windows Flags
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoDecoration;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_NoNav;
			window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
			window_flags |= ImGuiWindowFlags_MenuBar;

			{
				// Entites List Window
				ImGui::SetNextWindowPos(ImVec2(work_size.x - PAD, PAD), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
				ImGui::SetNextWindowSize(ImVec2(300.0f, (work_size.y / 2) - (PAD * 1.5f)), ImGuiCond_Always);

				ImGui::SetNextWindowBgAlpha(0.9f);

				if (ImGui::Begin("Entities", &p_open, window_flags)) {
					if (ImGui::BeginMenuBar()) {
						if (ImGui::BeginMenu("File")) {
							if (ImGui::MenuItem("New")) { Game::scene.newScene(); }
							if (ImGui::MenuItem("Save")) { Game::scene.saveScene(); }
							if (ImGui::MenuItem("Load")) { Game::scene.loadScene(); }
							ImGui::EndMenu();
						}

						if (ImGui::BeginMenu("Create")) {
							if (ImGui::MenuItem("Create Sprite")) { Game::scene.CreateSprite(); selectedEntity = Game::scene.entities.back(); }
							if (ImGui::MenuItem("Create Player")) { Game::scene.CreatePlayer(); selectedEntity = Game::scene.entities.back(); }
							ImGui::EndMenu();
						}
						ImGui::EndMenuBar();
					}

					if (ImGui::ListBoxHeader("Entities", ImVec2(300.0f, ((work_size.y / 2) - (PAD * 6))))) {
						for (auto const& entity : Game::scene.entities) {
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

					ImGui::End();
				}
			}
		}

		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 work_size = viewport->WorkSize;

			// Set Windows Flags
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoDecoration;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_NoNav;
			window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;

			// Toolbar Window
			ImGui::SetNextWindowPos(ImVec2(work_size.x - PAD, work_size.y - PAD), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
			ImGui::SetNextWindowSize(ImVec2(300.0f, (work_size.y / 2) - (PAD * 1.5f)), ImGuiCond_Always);

			ImGui::SetNextWindowBgAlpha(0.9f);

			if (ImGui::Begin("Toolbar", &p_open, window_flags)) {
				if (selectedEntity != nullptr) {
					selectedEntity->editmodeRender();
				}

				ImGui::End();
			}
		}
		ImGui::PopStyleVar();

		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
private:
	glm::vec2 velocity;
	float speed = 2;

	std::shared_ptr<Entity> selectedEntity;
};