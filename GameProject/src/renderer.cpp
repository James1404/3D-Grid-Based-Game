#include "renderer.h"

#include "window.h"

#include <memory>
#include <map>
#include <stack>

//
// ----- RENDERER -----
//

SDL_GLContext context;

glm::mat4 projection_matrix;
glm::mat4 view_matrix = glm::mat4(1.0f);

int screen_resolution_x = 1280, screen_resolution_y = 720;
float near_clip_plane = 0.1f, far_clip_plane = 100.0f;

static std::vector<model_entity_t*> model_list;

void init_renderer() {
	log_info("STARTING RENDERER INITIALIZATION");
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	context = SDL_GL_CreateContext(window);

	if (context != NULL) {
		glewExperimental = GL_TRUE;
		GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			log_error("Error initializing GLEW! ", glewGetErrorString(glewError));
		}
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	projection_matrix = glm::perspective(glm::radians(45.0f), (float)screen_resolution_x / (float)screen_resolution_y, near_clip_plane, far_clip_plane);

	log_info("SUCCESFULY COMPLETED RENDERER INITIALIZATION");

	// Set aspect ratio
	glViewport(0, 0, screen_resolution_x, screen_resolution_y);

	init_primitives();
}

void shutdown_renderer() {
	shutdown_primitives();

	SDL_GL_DeleteContext(context);
}

void renderer_clear_screen() {
	// Clear screen
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_swap_screen_buffers() {
	SDL_GL_SwapWindow(window);
}

void renderer_draw() {
	for (auto& i : model_list) {
		i->draw();
	}

	draw_primitives();
}

void model_entity_t::define_model(std::string _model_path,std::string _texture_path, glm::vec3* _position, glm::vec3* _rotation, glm::vec3* _scale)
{
	position = _position;
	rotation = _rotation;
	scale = _scale;

	is_paused = false;

	model = asset_manager.load_model_from_file(_model_path);
	shader = asset_manager.load_shader_from_file("data/shaders/model_loading.glsl");
	texture = asset_manager.load_texture_from_file(_texture_path);

	glUseProgram(shader->id);
	//glUniformMatrix4fv(glGetUniformLocation(shader->id, "projection"), 1, false, glm::value_ptr(projection_matrix));
	glUniform1i(glGetUniformLocation(shader->id, "Texture"), 0);
}

model_entity_t::model_entity_t()
{
	model_list.push_back(this);
}

model_entity_t::~model_entity_t() {
	for (auto it = model_list.begin(); it != model_list.end();) {
		if (*it == this)
			it = model_list.erase(it);
		else
			++it;
	}
}

void model_entity_t::draw() {
	if (is_paused)
		return;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	glUseProgram(shader->id);

	glm::mat4 new_model = glm::mat4(1.0f);
	new_model = glm::translate(new_model, *position);

	new_model = glm::rotate(new_model, glm::radians(rotation->x), glm::vec3(1, 0, 0));
	new_model = glm::rotate(new_model, glm::radians(rotation->y), glm::vec3(0, 1, 0));
	new_model = glm::rotate(new_model, glm::radians(rotation->z), glm::vec3(0, 0, 1));

	new_model = glm::scale(new_model, *scale);

	auto mvp = projection_matrix * view_matrix * new_model;

	glUniformMatrix4fv(glGetUniformLocation(shader->id, "mvp"), 1, false, glm::value_ptr(mvp));
	//glUniformMatrix4fv(glGetUniformLocation(shader->id, "view"), 1, false, glm::value_ptr(view_matrix));
	//glUniformMatrix4fv(glGetUniformLocation(shader->id, "model"), 1, false, glm::value_ptr(new_model));

#ifdef _DEBUG
	glUniform1i(glGetUniformLocation(shader->id, "entity_index"), index);
#endif

	model->draw(*shader);
}

//
// ----- PRIMITIVES -----
//

static std::shared_ptr<shader_t> line_shader;
static unsigned int line_vao, line_vbo, line_ebo;

void init_primitives()
{
	line_shader = asset_manager.load_shader_from_file("data/shaders/debug/debug_line.glsl");

	glGenVertexArrays(1, &line_vao);
	glBindVertexArray(line_vao);

	glGenBuffers(1, &line_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

	glGenBuffers(1, &line_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_ebo);
}

void shutdown_primitives()
{
	glDeleteProgram(line_shader->id);
	glDeleteBuffers(1, &line_vbo);
	glDeleteBuffers(1, &line_ebo);
	glDeleteVertexArrays(1, &line_vao);
}

static void renderer_draw_line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 colour) {
	// SETUP STUFF
	float vertices[] = {
		p1.x, p1.y, p1.z,	// first point
		p2.x, p2.y, p2.z	// second point
	};
	/*
	auto drawing = std::make_shared<debug_drawing>();

	drawing->colour = colour;

	*/
	//drawing->draw();

	glBindVertexArray(line_vao);
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glUseProgram(line_shader->id);

	glUniformMatrix4fv(glGetUniformLocation(line_shader->id, "u_projection"), 1, false, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(glGetUniformLocation(line_shader->id, "u_view"), 1, false, glm::value_ptr(view_matrix));

	glUniform3fv(glGetUniformLocation(line_shader->id, "u_color"), 1, glm::value_ptr(colour));

	glBindVertexArray(line_vao);
	glDrawArrays(GL_LINES, 0, 2);
	//line_draw_list.push_back(drawing);
}

static void renderer_draw_box_wireframe(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour) {
	glm::vec3 new_pos = pos + -.5f;
	renderer_draw_line(new_pos, { new_pos.x + size.x, new_pos.y, new_pos.z }, colour);
	renderer_draw_line(new_pos, { new_pos.x, new_pos.y + size.y, new_pos.z }, colour);
	renderer_draw_line(new_pos, { new_pos.x, new_pos.y, new_pos.z + size.z }, colour);

	renderer_draw_line(new_pos + size, new_pos + size - glm::vec3(size.x, 0, 0), colour);
	renderer_draw_line(new_pos + size, new_pos + size - glm::vec3(0, size.y, 0), colour);
	renderer_draw_line(new_pos + size, new_pos + size - glm::vec3(0, 0, size.z), colour);

	renderer_draw_line(new_pos + glm::vec3(0, size.y, 0), new_pos + size - glm::vec3(size.x, 0, 0), colour);
	renderer_draw_line(new_pos + glm::vec3(0, size.y, 0), new_pos + size - glm::vec3(0, 0, size.z), colour);

	renderer_draw_line(new_pos + size - glm::vec3(0, size.y, 0), new_pos + size - glm::vec3(size.x, size.y, 0), colour);
	renderer_draw_line(new_pos + size - glm::vec3(0, size.y, 0), new_pos + size - glm::vec3(0, size.y, size.z), colour);

	renderer_draw_line(new_pos + glm::vec3(size.x, 0, 0), new_pos + glm::vec3(size.x, size.y, 0), colour);
	renderer_draw_line(new_pos + glm::vec3(0, 0, size.z), new_pos + glm::vec3(0, size.y, size.z), colour);
}

enum class primitive_data_type { line, wireframe_cube };
struct primitive_data
{
	primitive_data_type type;

	struct
	{
		glm::vec3 pos, size;
	} wireframe_cube;
	struct
	{
		glm::vec3 p1, p2;
	} line;

	glm::vec3 colour;

	static primitive_data new_primitive_data(primitive_data_type type)
	{
		primitive_data data;
		data.type = type;
		return data;
	}
};

std::stack<primitive_data> primitive_draw_stack;

void draw_primitives()
{
	while (!primitive_draw_stack.empty())
	{
		const auto& primitive = primitive_draw_stack.top();

		switch (primitive.type)
		{
		case primitive_data_type::line:
			renderer_draw_line(primitive.line.p1, primitive.line.p2, primitive.colour);
			break;
		case primitive_data_type::wireframe_cube:
			renderer_draw_box_wireframe(primitive.wireframe_cube.pos, primitive.wireframe_cube.size, primitive.colour);
			break;
		}

		primitive_draw_stack.pop();
	}
}

void add_primitive_line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 colour)
{
	auto data = primitive_data::new_primitive_data(primitive_data_type::line);
	data.line.p1 = p1;
	data.line.p2 = p2;
	data.colour = colour;
	primitive_draw_stack.push(data);
}

void add_primitive_wireframe_cube(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour)
{
	auto data = primitive_data::new_primitive_data(primitive_data_type::wireframe_cube);
	data.wireframe_cube.pos = pos;
	data.wireframe_cube.size = size;
	data.colour = colour;
	primitive_draw_stack.push(data);
}
