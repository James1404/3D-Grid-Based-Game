#include "renderer.h"

#include "window.h"

#include <memory>
#include <map>
#include <stack>

//
// MODEL INSTANCE DATA
//

instance_container_t::~instance_container_t()
{
	// DELETE VBO AND OTHER STUFF
}

void instance_container_t::construct_instance_buffers()
{
	instance_buffer_size = instance_data.size();

	glBindVertexArray(model->vao);

	glGenBuffers(1, &instance_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);

#if EDITOR
	glBufferData(GL_ARRAY_BUFFER, (sizeof(glm::mat4) + sizeof(int)) * instance_buffer_size, NULL, GL_DYNAMIC_DRAW);
#else
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * instance_buffer_size, NULL, GL_DYNAMIC_DRAW);
#endif

	std::vector<glm::mat4> transforms;
#if EDITOR
	std::vector<int> indexs;
#endif // EDITOR
	for(int i = 0; i < instance_buffer_size; i++)
	{
		transforms.push_back(instance_data[i].transform->get_matrix());
#if EDITOR
		indexs.push_back(instance_data[i].index);
#endif // EDITOR
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * transforms.size(), &transforms[0]);
#if EDITOR
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * transforms.size(), sizeof(int) * indexs.size(), &indexs[0]);
#endif // EDITOR

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)sizeof(glm::vec4));

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * 2));

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * 3));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

#if EDITOR
	glEnableVertexAttribArray(7);
	glVertexAttribIPointer(7, 1, GL_INT, sizeof(int), (void*)(sizeof(glm::mat4) * transforms.size()));
	glVertexAttribDivisor(7, 1);
#endif // EDITOR

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

//
// RENDERER
//

void renderer_t::add_instance(std::string model_path, std::string texture_path, std::string shader_path, bool is_static, transform_t* transform, int index)
{
	// TODO: Instead of reconstructing buffer every time we add or remove one, we should
	// allocate more than we need to reduce the number of glBufferData calls.
	
	model_instance_data_t data = {transform, index};

	auto model = asset_manager_t::get().load_model_from_file(model_path);
	auto texture = asset_manager_t::get().load_texture_from_file(texture_path);
	auto shader = asset_manager_t::get().load_shader_from_file(shader_path);
	for(auto& container : instance_containers)
	{
		if(container.model == model && container.texture == texture && container.shader == shader && container.is_static == is_static)
		{
			container.instance_data.push_back(data);
			return;
		}
	}

	// it seems that the first model_instance isnt used for some reason???
	//
	// Instance container is being added to the containers list.
	// Draw call is being called....? (Thats a stupid way of writing it)
	// Model, Texture, and Shader are all loaded and used.
	//
	// It might be because one model is being used with multiple different textures and shaders.
	// (Maybe....???)
	//
	// FIXED IT!!!!!
	// (I just needed to redo the glVertexAttribPointer every frame after binding instance_vbo).

	instance_container_t container;
	container.model = model;
	container.texture = texture;
	container.shader = shader;
	container.is_static = is_static;

	glUseProgram(container.shader->id);
	glUniform1i(glGetUniformLocation(container.shader->id, "texture"), 0);

	container.instance_data.push_back(data);
	instance_containers.push_back(container);
}

void renderer_t::construct_all_instance_buffers()
{
	for(auto& container : instance_containers)
	{
		container.construct_instance_buffers();
	}
}

void renderer_t::update_non_static_instance_buffers()
{
	for(auto& container : instance_containers)
	{
		if(container.is_static)
		{
			continue;
		}

		std::vector<glm::mat4> transforms;

		for(int i = 0; i < container.instance_buffer_size; i++)
		{
			transforms.push_back(container.instance_data[i].transform->get_matrix());
		}

		glBindBuffer(GL_ARRAY_BUFFER, container.instance_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * transforms.size(), &transforms[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void renderer_t::update_all_instance_buffers()
{
	for(auto& container : instance_containers)
	{
		std::vector<glm::mat4> transforms;

		for(int i = 0; i < container.instance_buffer_size; i++)
		{
			transforms.push_back(container.instance_data[i].transform->get_matrix());
		}

		glBindBuffer(GL_ARRAY_BUFFER, container.instance_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * transforms.size(), &transforms[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

std::shared_ptr<shader_t> shader;
std::shared_ptr<texture_t> texture;

void renderer_t::init()
{
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

	projection_matrix = glm::perspective(glm::radians(60.0f), (float)screen_resolution_x / (float)screen_resolution_y, near_clip_plane, far_clip_plane);

	// Set aspect ratio
	glViewport(0, 0, screen_resolution_x, screen_resolution_y);

	primitive_renderer.init();

	log_info("Initialized Renderer");
}

void renderer_t::shutdown()
{
	instance_containers.clear();

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
	for(auto& container : instance_containers)
	{
		// BIND TEXTURES AND SHADERS
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, container.texture->id);

		glUseProgram(container.shader->id);

		auto view_projection = renderer_t::get().projection_matrix * renderer_t::get().view_matrix;

		glUniformMatrix4fv(glGetUniformLocation(container.shader->id, "view_projection"), 1, false, glm::value_ptr(view_projection));

		// BIND BUFFERS
		glBindVertexArray(container.model->vao);
		glBindBuffer(GL_ARRAY_BUFFER, container.instance_vbo);

		// UPDATE VERTEX ATTRIB POINTERS
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)sizeof(glm::vec4));

		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * 2));

		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * 3));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

#if EDITOR
		glEnableVertexAttribArray(7);
		glVertexAttribIPointer(7, 1, GL_INT, sizeof(int), (void*)(sizeof(glm::mat4) * container.instance_buffer_size));
		glVertexAttribDivisor(7, 1);
#endif // EDITOR
		
		// DRAW MESH
		glDrawElementsInstanced(GL_TRIANGLES, container.model->indices.size(), GL_UNSIGNED_INT, 0, container.instance_buffer_size);

		// UNBIND BUFFERS
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
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
	line_shader = asset_manager_t::get().load_shader_from_file("data/shaders/debug_line.glsl");

	glGenVertexArrays(1, &line_vao);
	glBindVertexArray(line_vao);

	glGenBuffers(1, &line_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

	glGenBuffers(1, &line_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_ebo);
	
	// Quad
	quad_shader = asset_manager_t::get().load_shader_from_file("data/shaders/debug_quad.glsl");

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
	float vertices[] =
	{
		p1.x, p1.y, p1.z,	// first point
		p2.x, p2.y, p2.z	// second point
	};

	glBindVertexArray(line_vao);
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glUseProgram(line_shader->id);

	auto view_projection = renderer_t::get().projection_matrix * renderer_t::get().view_matrix;
	glUniformMatrix4fv(glGetUniformLocation(line_shader->id, "u_view_projection"), 1, false, glm::value_ptr(view_projection));

	glUniform3fv(glGetUniformLocation(line_shader->id, "u_color"), 1, glm::value_ptr(colour));

	glBindVertexArray(line_vao);
	glDrawArrays(GL_LINES, 0, 2);
}

void primitive_renderer_t::draw_box_wireframe(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour) {
	glm::vec3 new_pos = pos + -.5f;

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	draw_quad(new_pos, {new_pos.x + size.x, new_pos.y + size.y, new_pos.z},          colour);
	draw_quad(new_pos, {new_pos.x 		  , new_pos.y + size.y, new_pos.z + size.z}, colour);
	draw_quad({new_pos.x, new_pos.y + size.y, new_pos.z}, {new_pos.x + size.x, new_pos.y + size.y, new_pos.z + size.z}, colour);

	draw_quad(new_pos + size, new_pos + size - glm::vec3(size.x, size.y, 0), colour);
	draw_quad(new_pos + size, new_pos + size - glm::vec3(0, size.y, size.x), colour);
	draw_quad(new_pos + size - glm::vec3(0, size.y, 0), new_pos, colour);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void primitive_renderer_t::draw_quad(const glm::vec3 min, const glm::vec3 max, const glm::vec3 colour)
{
	// SETUP STUFF
	float vertices[] =
	{
		max.x, max.y, max.z, // top right
		max.x, min.y, max.z, // bottom right
		min.x, min.y, min.z, // bottom left
		min.x, max.y, min.z  // top left
	};

	glBindVertexArray(quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	unsigned int indices[] =
	{
		0, 1, 3,
		1, 2, 3
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glUseProgram(quad_shader->id);

	auto view_projection = renderer_t::get().projection_matrix * renderer_t::get().view_matrix;

	glUniformMatrix4fv(glGetUniformLocation(quad_shader->id, "u_view_projection"), 1, false, glm::value_ptr(view_projection));

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
		glm::vec3 min, max;
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
			draw_quad(primitive.quad.min, primitive.quad.max, primitive.colour);
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

void primitive_renderer_t::add_quad(const glm::vec3 min, const glm::vec3 max, const glm::vec3 colour)
{
	auto data = primitive_data::new_primitive_data(primitive_data_type::quad);
	data.quad.min = min;
	data.quad.max = max;
	data.colour = colour;
	primitive_draw_stack.push(data);
}
