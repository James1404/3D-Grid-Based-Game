#include "renderer.h"

#include <memory>
#include <map>

//
// ----- RENDERER -----
//

SDL_Window* renderer::window;
SDL_GLContext renderer::context;

glm::mat4 renderer::projection;
glm::mat4 renderer::view = glm::mat4(1.0f);

int renderer::screen_resolution_x = 1280, renderer::screen_resolution_y = 720;

asset_manager_t renderer::asset_manager;

static std::vector<renderer::model_entity_t*> model_list;

void renderer::init() {
	log_info("STARTING RENDERER INITIALIZATION");
	
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_resolution_x, screen_resolution_y, SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE*/);
		context = SDL_GL_CreateContext(window);


		if (context != NULL) {
			glewExperimental = GL_TRUE;
			GLenum glewError = glewInit();
			if (glewError != GLEW_OK) {
				log_error("Error initializing GLEW! ", glewGetErrorString(glewError));
			}
		}
	}

	glEnable(GL_DEPTH_TEST);

	projection = glm::perspective(glm::radians(45.0f), (float)screen_resolution_x / (float)screen_resolution_y, 0.1f, 100.0f);

	log_info("SUCCESFULY COMPLETED RENDERER INITIALIZATION");
}

void renderer::clean() {
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void renderer::start_drawing_frame() {
	// Clear screen
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set aspect ratio
	glViewport(0, 0, screen_resolution_x, screen_resolution_y);
}

void renderer::draw_models() {
	for (auto& i : model_list) {
		i->draw();
	}
}

void renderer::stop_drawing_frame() {
	SDL_GL_SwapWindow(window);
}

renderer::model_entity_t::model_entity_t(std::string _model_path,std::string _texture_path, glm::vec3* _position)
{
	position = _position;
	rotation = glm::vec3(0);
	scale = glm::vec3(1);

	is_paused = false;

	model = asset_manager.load_model_from_file(_model_path);
	shader = asset_manager.load_shader_from_file("data/shaders/model_loading.glsl");
	texture = asset_manager.load_texture_from_file(_texture_path);

	glUseProgram(shader->id);
	glUniformMatrix4fv(glGetUniformLocation(shader->id, "projection"), 1, false, glm::value_ptr(projection));
	glUniform1i(glGetUniformLocation(shader->id, "Texture"), 0);

	model_list.push_back(this);
}

renderer::model_entity_t::~model_entity_t() {
	for (auto it = model_list.begin(); it != model_list.end();) {
		if (*it == this)
			it = model_list.erase(it);
		else
			++it;
	}
}

void renderer::model_entity_t::draw() {
	if (is_paused)
		return;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	glUseProgram(shader->id);

	glm::mat4 new_model = glm::mat4(1.0f);
	new_model = glm::translate(new_model, *position);
	//new_model = glm::rotate(new_model, rotation);
	new_model = glm::rotate(new_model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	new_model = glm::rotate(new_model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	new_model = glm::rotate(new_model, glm::radians(rotation.z), glm::vec3(0, 0, 1));

	new_model = glm::scale(new_model, scale);

	glUniformMatrix4fv(glGetUniformLocation(shader->id, "view"), 1, false, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader->id, "model"), 1, false, glm::value_ptr(new_model));

	model->draw(*shader);
}

#ifdef _DEBUG
//
// ----- DEBUG RENDERER -----
//

renderer::debug::debug_drawing::debug_drawing() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

renderer::debug::debug_drawing::~debug_drawing() {
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}

std::shared_ptr<shader_t> line_shader;
void renderer::debug::init_debug() {
	line_shader	= asset_manager.load_shader_from_file("data/shaders/debug/debug_line.shader");
}

void renderer::debug::clean_debug() {
	clear_debug_list();
	glDeleteProgram(line_shader->id);
}

std::vector<std::shared_ptr<renderer::debug::debug_drawing>> line_draw_list;

void renderer::debug::clear_debug_list () {
	line_draw_list.clear();
}

void renderer::debug::draw_debug() {
	for(auto& line : line_draw_list) {
		glUseProgram(line_shader->id);

		glUniformMatrix4fv(glGetUniformLocation(line_shader->id, "u_projection"), 1, false, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(line_shader->id, "u_view"), 1, false, glm::value_ptr(view));

		glUniform3fv(glGetUniformLocation(line_shader->id, "u_color"), 1, glm::value_ptr(line->colour));

		glBindVertexArray(line->vao);
		glDrawArrays(GL_LINES, 0, 2);
	}
}

void renderer::debug::draw_line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 colour) {
	// SETUP STUFF
	float vertices[] = {
		p1.x, p1.y, p1.z,	// first point
		p2.x, p2.y, p2.z	// second point
	};

	auto drawing = std::make_shared<debug_drawing>();

	drawing->colour = colour;

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	line_draw_list.push_back(drawing);
}

void renderer::debug::draw_box_wireframe(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour) {
	glm::vec3 new_pos = pos + -.5f;
	draw_line(new_pos, { new_pos.x + size.x, new_pos.y, new_pos.z }, colour);
	draw_line(new_pos, { new_pos.x, new_pos.y + size.y, new_pos.z }, colour);
	draw_line(new_pos, { new_pos.x, new_pos.y, new_pos.z + size.z }, colour);

	draw_line(new_pos + size, new_pos + size - glm::vec3(size.x, 0, 0), colour);
	draw_line(new_pos + size, new_pos + size - glm::vec3(0, size.y, 0), colour);
	draw_line(new_pos + size, new_pos + size - glm::vec3(0, 0, size.z), colour);

	draw_line(new_pos + glm::vec3(0, size.y, 0), new_pos + size - glm::vec3(size.x, 0, 0), colour);
	draw_line(new_pos + glm::vec3(0, size.y, 0), new_pos + size - glm::vec3(0, 0, size.z), colour);

	draw_line(new_pos + size - glm::vec3(0, size.y, 0), new_pos + size - glm::vec3(size.x, size.y, 0), colour);
	draw_line(new_pos + size - glm::vec3(0, size.y, 0), new_pos + size - glm::vec3(0, size.y, size.z), colour);

	draw_line(new_pos + glm::vec3(size.x, 0, 0), new_pos + glm::vec3(size.x, size.y, 0), colour);
	draw_line(new_pos + glm::vec3(0, 0, size.z), new_pos + glm::vec3(0, size.y, size.z), colour);
}
#else
renderer::debug::debug_drawing::debug_drawing()
{}

renderer::debug::debug_drawing::~debug_drawing()
{}

void renderer::debug::init_debug()
{}

void renderer::debug::clean_debug()
{}

void renderer::debug::clear_debug_list()
{}

void renderer::debug::draw_debug()
{}

void renderer::debug::draw_line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 colour)
{}

void renderer::debug::draw_box_wireframe(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour)
{}
#endif // _DEBUG
