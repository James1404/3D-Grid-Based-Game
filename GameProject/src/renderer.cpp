#include "renderer.h"

#include <memory>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "stb_image.h"

//
// ----- SHADERS -----
//

void check_shader_compiler_errors(unsigned int shader) {
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("ERROR::SHADER::COMPILATION_FAILED%.4s\n", infoLog);
	}
}

unsigned int create_shader(const char* vertexSource, const char* fragmentSource) {
	// load shaders from file
	std::string vShaderCode, fShaderCode;
	std::ifstream vShaderFile, fShaderFile;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vShaderFile.open(vertexSource);
		fShaderFile.open(fragmentSource);
		std::stringstream vShaderStream, fShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vShaderCode = vShaderStream.str();
		fShaderCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		printf("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n");
	}

	const char* vertexShader = vShaderCode.c_str();
	const char* fragmentShader = fShaderCode.c_str();

	unsigned int vertex, fragment;

	//create and compiler shaders from its source
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexShader, NULL);
	glCompileShader(vertex);
	check_shader_compiler_errors(vertex);

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentShader, NULL);
	glCompileShader(fragment);
	check_shader_compiler_errors(fragment);

	// create program and attach shaders
	unsigned int shader;
	shader = glCreateProgram();
	glAttachShader(shader, vertex);
	glAttachShader(shader, fragment);
	glLinkProgram(shader);

	// delete shaders as they are no longer needed
	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return shader;
}

//
// ----- RENDERER -----
//

SDL_Window* renderer::window;
SDL_GLContext renderer::context;

glm::mat4 renderer::projection;
glm::mat4 renderer::view = glm::mat4(1.0f);

int renderer::screen_width = 1280, renderer::screen_height = 720;
const int renderer::screen_resolution_x = 320, renderer::screen_resolution_y = 200;

static std::vector<renderer::sprite*> render_list;

unsigned int sprite_shader;

void renderer::init() {
	printf("STARTING RENDERER INITIALIZATION\n");
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	context = SDL_GL_CreateContext(window);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (context != NULL) {
		glewExperimental = GL_TRUE;
		GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
		}
	}

	projection = glm::ortho(0.0f, (float)screen_resolution_x, 0.0f, (float)screen_resolution_y, -100.0f, 100.0f);

	sprite_shader = create_shader("data/shaders/core.vs", "data/shaders/core.fs");

	printf("SUCCESFULY COMPLETED RENDERER INITIALIZATION\n");
}

void renderer::clean() {
	glDeleteProgram(sprite_shader);

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void renderer::start_draw() {
	// Clear screen
	glClearColor(0.0, 0.0, 0.0, 0.0);
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
}

void renderer::draw_sprites() {
	std::multimap <int, sprite*> render_order;
	for (auto& i : render_list) {
		render_order.insert({ i->layer, i });
	}

	for (auto& i : render_order) {
		i.second->draw();
	}
}

void renderer::stop_draw() {
	SDL_GL_SwapWindow(window);
}

//
// ----- SPRITES -----
//

renderer::sprite::sprite() {
	// Generate and bind buffers
	float vertices[] = {
		// positions
		 1.0f,  1.0f, // top right
		 1.0f,  0.0f, // bottom right
		 0.0f,  0.0f, // bottom left
		 0.0f,  1.0f, // top left
	};

	float texcoords[] = {
		// texture coords
		1.0f, 1.0f, // top right
		1.0f, 0.0f, // bottom right
		0.0f, 0.0f, // bottom left
		0.0f, 1.0f  // top left
	};

	// Generate and bind vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Generate and bind vbo
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(texcoords), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texcoords), &texcoords);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(vertices)));

	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Set image attribute
	// glUniform1i(glGetUniformLocation(sprite_shader, "image"), 0);

	// Set projection matrix attribute
	glUseProgram(sprite_shader);
	glUniformMatrix4fv(glGetUniformLocation(sprite_shader, "projection"), 1, false, glm::value_ptr(projection));

	render_list.push_back(this);
}

renderer::sprite::~sprite() {
	// glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &vao);

	for (auto it = render_list.begin(); it != render_list.end();) {
		if (*it == this)
			it = render_list.erase(it);
		else
			++it;
	}
}

void renderer::sprite::draw() {
	glUseProgram(sprite_shader);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(*position * (float)cell_size, 0));
	model = glm::scale(model, glm::vec3(cell_size, cell_size, 1.0f));

	glUniform3fv(glGetUniformLocation(sprite_shader, "colour"), 1, glm::value_ptr(colour));

	glUniformMatrix4fv(glGetUniformLocation(sprite_shader, "view"), 1, false, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(sprite_shader, "model"), 1, false, glm::value_ptr(model));

	/*
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	*/

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

/*
void renderer::sprite::set_sprite_path(std::string sprite_name) {
	std::string path = "data/textures/";
	path.append(sprite_name);

	// Load and Generate Textures
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_set_flip_vertically_on_load(true);

	int nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &size.x, &size.y, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
}
*/

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

unsigned int square_shader;
unsigned int circle_shader;
unsigned int line_shader;
void renderer::debug::init_debug() {
	square_shader = create_shader("data/shaders/square.vs", "data/shaders/square.fs");
	circle_shader = create_shader("data/shaders/circle.vs", "data/shaders/circle.fs");
	line_shader	= create_shader("data/shaders/line.vs", "data/shaders/line.fs");
}

void renderer::debug::clean_debug() {
	clear_debug_list();

	glDeleteProgram(square_shader);
	glDeleteProgram(circle_shader);
	glDeleteProgram(line_shader);
}

std::vector<std::shared_ptr<renderer::debug::debug_drawing>> square_draw_list;
std::vector<std::shared_ptr<renderer::debug::debug_drawing>> circle_draw_list;
std::vector<std::shared_ptr<renderer::debug::debug_drawing>> line_draw_list;

void renderer::debug::clear_debug_list () {
	square_draw_list.clear();
	circle_draw_list.clear();
	line_draw_list.clear();
}

void renderer::debug::draw_debug() {
	for(auto& square : square_draw_list) {
		glUseProgram(square_shader);

		glUniformMatrix4fv(glGetUniformLocation(square_shader, "u_projection"), 1, false, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(square_shader, "u_view"), 1, false, glm::value_ptr(view));
		glUniform1i(glGetUniformLocation(square_shader, "u_is_screen_space"), square->screen_space);

		glUniform3fv(glGetUniformLocation(square_shader, "u_color"), 1, glm::value_ptr(square->colour));

		glBindVertexArray(square->vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	for(auto& circle : circle_draw_list) {
		glUseProgram(circle_shader);

		glUniformMatrix4fv(glGetUniformLocation(circle_shader, "u_projection"), 1, false, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(circle_shader, "u_view"), 1, false, glm::value_ptr(view));
		glUniform1i(glGetUniformLocation(circle_shader, "u_is_screen_space"), circle->screen_space);

		glUniform3fv(glGetUniformLocation(circle_shader, "u_color"), 1, glm::value_ptr(circle->colour));

		glBindVertexArray(circle->vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	for(auto& line : line_draw_list) {
		glUseProgram(line_shader);

		glUniformMatrix4fv(glGetUniformLocation(line_shader, "u_projection"), 1, false, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(line_shader, "u_view"), 1, false, glm::value_ptr(view));
		glUniform1i(glGetUniformLocation(line_shader, "u_is_screen_space"), line->screen_space);

		glUniform3fv(glGetUniformLocation(line_shader, "u_color"), 1, glm::value_ptr(line->colour));

		glBindVertexArray(line->vao);
		glDrawArrays(GL_LINES, 0, 2);
	}
}

void renderer::debug::draw_line(const glm::vec2 p1, const glm::vec2 p2, const glm::vec3 colour, bool screen_space) {
	// SETUP STUFF
	float vertices[] = {
		p1.x, p1.y,	// first point
		p2.x, p2.y	// second point
	};

	auto drawing = std::make_shared<debug_drawing>();

	drawing->screen_space = screen_space;
	drawing->colour = colour;

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	line_draw_list.push_back(drawing);
}

void renderer::debug::draw_box(const glm::vec2 position, const glm::vec2 size, const glm::vec3 colour, bool screen_space) {
	// SETUP STUFF
	float vertices[] = {
		position.x + size.x, position.y + size.y,	// top right
		position.x + size.x, position.y,			// bottom right
		position.x,		     position.y,			// bottom left
		position.x,		     position.y + size.y,	// top left
	};
	
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	auto drawing = std::make_shared<debug_drawing>();

	drawing->screen_space = screen_space;
	drawing->colour = colour;

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	square_draw_list.push_back(drawing);
}

void renderer::debug::draw_box_wireframe(const glm::vec2 pos, const glm::vec2 size, const glm::vec3 colour, bool screen_space) {
	draw_line(pos, { size.x + pos.x, pos.y }, colour);
	draw_line(pos, { pos.x, size.y + pos.y }, colour);
	draw_line(size + pos, { pos.x, size.y + pos.y }, colour);
	draw_line(size + pos, { size.x + pos.x, pos.y }, colour);
}

void renderer::debug::draw_circle(const glm::vec2 position, const float radius, const glm::vec3 colour, bool screen_space) {
	// SETUP STUFF
	float vertices[] = {
		position.x + (radius * .5f), position.y + (radius * .5f),	// top right
		position.x + (radius * .5f), position.y - (radius * .5f),	// bottom right
		position.x - (radius * .5f), position.y - (radius * .5f),	// bottom left
		position.x - (radius * .5f), position.y + (radius * .5f)	// top left
	};

	float texcoords[] = {
		// texture coords
		1.0f, 1.0f, // top right
		1.0f, 0.0f, // bottom right
		0.0f, 0.0f, // bottom left
		0.0f, 1.0f  // top left
	};

	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	auto drawing = std::make_shared<debug_drawing>();

	drawing->screen_space = screen_space;
	drawing->colour = colour;

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(texcoords), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texcoords), &texcoords);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(vertices)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	circle_draw_list.push_back(drawing);
}
#endif // _DEBUG
