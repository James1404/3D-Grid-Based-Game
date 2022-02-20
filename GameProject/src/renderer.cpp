#include "renderer.h"

#include "window.h"

#include <memory>
#include <map>
#include <stack>

//
// RENDERER
//

std::shared_ptr<shader_t> shader;
std::shared_ptr<texture_t> texture;

void renderer_t::init()
{
	//log_info("STARTING RENDERER INITIALIZATION");
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	context = SDL_GL_CreateContext(window_t::get().window);

	if (context != NULL)
	{
		glewExperimental = GL_TRUE;
		GLenum glewError = glewInit();
		if (glewError != GLEW_OK)
		{
			log_error("Error initializing GLEW! ", glewGetErrorString(glewError));
		}
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	SDL_GL_SetSwapInterval(1);

	projection_matrix = glm::perspective(glm::radians(45.0f), (float)screen_resolution_x / (float)screen_resolution_y, near_clip_plane, far_clip_plane);


	// Set aspect ratio
	glViewport(0, 0, screen_resolution_x, screen_resolution_y);

	primitive_renderer.init();

	log_info("INITIALIZED RENDERER");

	shader = asset_manager_t::get().load_shader_from_file("data/shaders/model_loading.glsl");
	texture = asset_manager_t::get().load_texture_from_file("data/models/diffuse.jpg");

	glUseProgram(shader->id);
	glUniform1i(glGetUniformLocation(shader->id, "texture"), 0);
}

void renderer_t::shutdown()
{
	primitive_renderer.shutdown();

	SDL_GL_DeleteContext(context);
}

void renderer_t::clear_screen()
{
	// Clear screen
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_t::swap_screen_buffers()
{
	SDL_GL_SwapWindow(window_t::get().window);
}

void renderer_t::draw()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	glUseProgram(shader->id);

	auto view_projection = renderer_t::get().projection_matrix * renderer_t::get().view_matrix;

	glUniformMatrix4fv(glGetUniformLocation(shader->id, "view_projection"), 1, false, glm::value_ptr(view_projection));

	for(auto& [path, model] : asset_manager_t::get().models)
	{
		model->draw();
	}

	primitive_renderer.draw();
}

void renderer_t::set_resolution(int x, int y)
{
	screen_resolution_x = x; 
	screen_resolution_y = y;
	glViewport(0, 0, screen_resolution_x, screen_resolution_y);
	projection_matrix = glm::perspective(glm::radians(45.0f), (float)screen_resolution_x / (float)screen_resolution_y, near_clip_plane, far_clip_plane);
}

//
// ----- PRIMITIVES -----
//

void primitive_renderer_t::init()
{
	// Line
	line_shader = asset_manager_t::get().load_shader_from_file("data/shaders/debug/debug_line.glsl");

	glGenVertexArrays(1, &line_vao);
	glBindVertexArray(line_vao);

	glGenBuffers(1, &line_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

	glGenBuffers(1, &line_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_ebo);

	// Quad
	quad_shader = asset_manager_t::get().load_shader_from_file("data/shaders/debug/debug_quad.glsl");

	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

	glGenBuffers(1, &quad_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
}

void primitive_renderer_t::shutdown()
{
	// Line
	glDeleteProgram(line_shader->id);
	glDeleteBuffers(1, &line_vbo);
	glDeleteBuffers(1, &line_ebo);
	glDeleteVertexArrays(1, &line_vao);
	
	// Quad
	glDeleteProgram(quad_shader->id);
	glDeleteBuffers(1, &quad_vbo);
	glDeleteBuffers(1, &quad_ebo);
	glDeleteVertexArrays(1, &quad_vao);
}

void primitive_renderer_t::draw_line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 colour) {
	// SETUP STUFF
	float vertices[] = {
		p1.x, p1.y, p1.z,	// first point
		p2.x, p2.y, p2.z	// second point
	};

	glBindVertexArray(line_vao);
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glUseProgram(line_shader->id);

	glUniformMatrix4fv(glGetUniformLocation(line_shader->id, "u_projection"), 1, false, glm::value_ptr(renderer_t::get().projection_matrix));
	glUniformMatrix4fv(glGetUniformLocation(line_shader->id, "u_view"), 1, false, glm::value_ptr(renderer_t::get().view_matrix));

	glUniform3fv(glGetUniformLocation(line_shader->id, "u_color"), 1, glm::value_ptr(colour));

	glBindVertexArray(line_vao);
	glDrawArrays(GL_LINES, 0, 2);
	//line_draw_list.push_back(drawing);
}

void primitive_renderer_t::draw_box_wireframe(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour) {
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

void primitive_renderer_t::draw_quad(const glm::vec3 pos, const glm::vec3 rot, const glm::vec2 scl, const glm::vec3 colour) {
	// SETUP STUFF
	float vertices[] =
	{
		 0.5f,  0.5f, // top right
		 0.5f, -0.5f, // bottom right
		-0.5f, -0.5f, // bottom left
		-0.5f,  0.5f  // top left
	};

	glBindVertexArray(quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	unsigned int indices[] =
	{
		0, 1, 3,
		1, 2, 3
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glUseProgram(quad_shader->id);

	auto model = glm::mat4(1.0f);
	model = glm::translate(model, pos);

	model = glm::rotate(model, glm::radians(rot.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(rot.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rot.z), glm::vec3(0, 0, 1));

	model = glm::scale(model, glm::vec3(scl, 0));

	auto mvp = renderer_t::get().projection_matrix * renderer_t::get().view_matrix * model;

	glUniformMatrix4fv(glGetUniformLocation(quad_shader->id, "u_mvp"), 1, false, glm::value_ptr(mvp));

	glUniform3fv(glGetUniformLocation(quad_shader->id, "u_color"), 1, glm::value_ptr(colour));

	glDisable(GL_CULL_FACE);

	glBindVertexArray(quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glEnable(GL_CULL_FACE);
}

enum class primitive_data_type { line, wireframe_cube, quad };
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

	struct
	{
		glm::vec3 pos, rot;
		glm::vec2 scl;
	} quad;

	glm::vec3 colour;

	static primitive_data new_primitive_data(primitive_data_type type)
	{
		primitive_data data;
		data.type = type;
		return data;
	}
};

std::stack<primitive_data> primitive_draw_stack;

void primitive_renderer_t::draw()
{
	while (!primitive_draw_stack.empty())
	{
		const auto& primitive = primitive_draw_stack.top();

		switch (primitive.type)
		{
		case primitive_data_type::line:
			draw_line(primitive.line.p1, primitive.line.p2, primitive.colour);
			break;
		case primitive_data_type::wireframe_cube:
			draw_box_wireframe(primitive.wireframe_cube.pos, primitive.wireframe_cube.size, primitive.colour);
			break;
		case primitive_data_type::quad:
			draw_quad(primitive.quad.pos, primitive.quad.rot, primitive.quad.scl, primitive.colour);
			break;
		}

		primitive_draw_stack.pop();
	}
}

void primitive_renderer_t::add_line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 colour)
{
	auto data = primitive_data::new_primitive_data(primitive_data_type::line);
	data.line.p1 = p1;
	data.line.p2 = p2;
	data.colour = colour;
	primitive_draw_stack.push(data);
}

void primitive_renderer_t::add_wireframe_cube(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour)
{
	auto data = primitive_data::new_primitive_data(primitive_data_type::wireframe_cube);
	data.wireframe_cube.pos = pos;
	data.wireframe_cube.size = size;
	data.colour = colour;
	primitive_draw_stack.push(data);
}

void primitive_renderer_t::add_quad(const glm::vec3 pos, const glm::vec3 rot, const glm::vec2 scl, const glm::vec3 colour)
{
	auto data = primitive_data::new_primitive_data(primitive_data_type::quad);
	data.quad.pos = pos;
	data.quad.rot = rot;
	data.quad.scl = scl;
	data.colour = colour;
	primitive_draw_stack.push(data);
}
