#include "Game.h"
#include "Entities/EditmodeCamera.h"
#include "Scene.h"

glm::mat4 Game::projection;
glm::mat4 Game::view = glm::mat4(1.0f);

GameState Game::gameState = GameState::GameState_Game;

SDL_Event Game::event;

int Game::screen_width = 1280;
int Game::screen_height = 720;

const int Game::screen_resolution_x = 320;
const int Game::screen_resolution_y = 200;

EditmodeCamera camera;
Scene scene;

std::shared_ptr<Entity> selectedEntity;

void Game::init(const char* title) {
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		
		const char* glsl_version = "#version 330";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		context = SDL_GL_CreateContext(window);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		if (context != NULL) {
			glewExperimental = GL_TRUE;
			GLenum glewError = glewInit();
			if (glewError != GLEW_OK) {
				printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
			}
		}

		projection = glm::ortho(0.0f, static_cast<float>(screen_resolution_x), 0.0f, static_cast<float>(screen_resolution_y), -1.0f, 1.0f);

		// Initialize Entites
		scene.init();

		// Initialize Imgui
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplSDL2_InitForOpenGL(window, context);
		ImGui_ImplOpenGL3_Init(glsl_version);

		isRunning = true;
	}
}

void Game::handleEvents() {
	if (SDL_PollEvent(&event)) {
		ImGui_ImplSDL2_ProcessEvent(&event);
		switch (event.type) {
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				screen_width = event.window.data1;
				screen_height = event.window.data2;
				SDL_SetWindowSize(window, screen_width, screen_height);
			}
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				if (gameState == GameState::GameState_Game) {
					gameState = GameState::GameState_Edit;
					break;
				}

				gameState = GameState::GameState_Game;
			}
			break;
		default:
			break;
		}
	}
}

Uint64 NOW = SDL_GetPerformanceCounter(), LAST = 0;
void Game::update() {
	// calculate delta time;
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();

	double dt = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

	if (gameState == GameState::GameState_Edit) {
		camera.update(dt);
	}
	else {
		scene.update(dt);
	}
}

void Game::render() {
	// Clear screen
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Set aspect ratio
	float x = screen_width / (float)screen_resolution_x;
	float y = screen_height / (float)screen_resolution_y;
	float aspect = std::min(x, y);

	int viewWidth = screen_resolution_x * aspect;
	int viewHeight = screen_resolution_y * aspect;

	int viewX = (screen_width - screen_resolution_x * aspect) / 2;
	int viewY = (screen_height - screen_resolution_y * aspect) / 2;

	glViewport(viewX, viewY, viewWidth, viewHeight);

	// render scene
	scene.render();

	// render imgui windows
	if (gameState == GameState::GameState_Edit) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 work_pos = viewport->WorkPos;
			ImVec2 work_size = viewport->WorkSize;
			ImVec2 window_pos, window_pos_pivot;

			const float PAD = 10.0f;

			window_pos.x = work_pos.x + PAD;
			window_pos.y = work_pos.y + PAD;
			window_pos_pivot.x = 0.0f;
			window_pos_pivot.y = 0.0f;

			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoDecoration;
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
			window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
			window_flags |= ImGuiWindowFlags_NoNav;
			window_flags |= ImGuiWindowFlags_MenuBar;

			static bool p_open = true;
			ImGui::SetNextWindowBgAlpha(0.9f);
			if (ImGui::Begin("Overlay", &p_open, window_flags)) {
				if (ImGui::BeginMenuBar()) {
					if (ImGui::BeginMenu("File")) {
						if (ImGui::MenuItem("New")) { scene.newScene(); }
						if (ImGui::MenuItem("Save")) { scene.saveScene(); }
						if (ImGui::MenuItem("Load")) { scene.loadScene(); }
						ImGui::EndMenu();
					}

					ImGui::EndMenuBar();
				}

				ImGui::Text("Game Stats");

				ImGui::Separator();
				ImGui::Text("Screen Size: (%i, %i)", screen_width, screen_height);
				ImGui::Text("No. of Entities: %i", scene.entities.size());

				ImGui::End();
			}
		}

		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();

			ImVec2 work_pos = viewport->WorkPos;
			ImVec2 work_size = viewport->WorkSize;

			const float PAD = 10.0f;

			// Set Windows Flags
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoDecoration;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_NoNav;
			window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;

			static bool p_open = NULL;

			{
				// Entites List Window
				ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - PAD, work_pos.y + PAD), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
				ImGui::SetNextWindowSize(ImVec2(300.0f, (work_size.y / 2) - work_pos.y - (PAD * 1.5f)), ImGuiCond_Always);

				ImGui::SetNextWindowBgAlpha(0.9f);

				if (ImGui::Begin("Entities", &p_open, window_flags)) {
					if (ImGui::BeginPopupContextWindow()) {
						if (ImGui::MenuItem("Create Sprite")) { scene.CreateSprite(); selectedEntity = scene.entities.back(); }
						if (ImGui::MenuItem("Create Player")) { scene.CreatePlayer(); selectedEntity = scene.entities.back(); }
						ImGui::EndPopup();
					}

					if (ImGui::ListBoxHeader("Entities", ImVec2(300.0f, (work_size.y / 4) + PAD))) {
						for (auto const& entity : scene.entities) {
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

			{
				// Toolbar Window
				ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - PAD, work_size.y - PAD), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
				ImGui::SetNextWindowSize(ImVec2(300.0f, (work_size.y / 2) + work_pos.y - (PAD * 1.5f)), ImGuiCond_Always);

				ImGui::SetNextWindowBgAlpha(0.9f);

				if (ImGui::Begin("Toolbar", &p_open, window_flags)) {
					if (selectedEntity != nullptr) {
						selectedEntity->editmodeRender();
					}

					ImGui::End();
				}
			}
		}
		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	// Swap buffer
	SDL_GL_SwapWindow(window);
}

void Game::clean() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}