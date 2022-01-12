#include "renderer.h"

#include <memory>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

//
// ----- MESH -----
//

renderer::Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	setupMesh();
}

void renderer::Mesh::draw(Shader& _shader) {
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);

		glUniform1i(glGetUniformLocation(_shader.id, (name + number).c_str()), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void renderer::Mesh::setupMesh() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

	glBindVertexArray(0);
}

//
// ----- MODEL -----
//


void renderer::Model::draw(Shader& _shader) {
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].draw(_shader);
	}
}

void renderer::Model::load_model(std::string _path) {
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(_path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		logger::error("ERROR::ASSIMP::", import.GetErrorString());
		return;
	}
	directory = _path.substr(0, _path.find_last_of('/'));

	process_node(scene->mRootNode, scene);
}

void renderer::Model::process_node(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(process_mesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		process_node(node->mChildren[i], scene);
	}
}

renderer::Mesh renderer::Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;

		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.tex_coords = vec;
		}
		else
			vertex.tex_coords = glm::vec2(0);

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = load_material_textures(material, aiTextureType_DIFFUSE, " texture_diffuse");

		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = load_material_textures(material, aiTextureType_SPECULAR, " texture_specular");

		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

std::vector<renderer::Texture> renderer::Model::load_material_textures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			Texture texture;
			texture.id = texture_from_file(str.C_Str(), directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
	return textures;
}

//
// ----- SHADERS -----
//

renderer::Shader::Shader(const char* vertexSource, const char* fragmentSource) {
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
		logger::error("SHADER FILE NOT SUCCESFULLY READ");
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

	logger::info("Loaded and compiled shader ", vertexSource, " ", fragmentSource);

	id = shader;
}

void renderer::Shader::check_shader_compiler_errors(unsigned int shader) {
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		logger::error("SHADER COMPILATION FAILED ", infoLog);
	}
}

void check_shader_compiler_errors(unsigned int shader) {
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		logger::error("SHADER COMPILATION FAILED ", infoLog);
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
		logger::error("SHADER FILE NOT SUCCESFULLY READ");
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

	logger::info("Loaded and compiled shader ", vertexSource, " ", fragmentSource);

	return shader;
}

//
// ----- RENDERER -----
//

SDL_Window* renderer::window;
SDL_GLContext renderer::context;

glm::mat4 renderer::projection;
glm::mat4 renderer::view = glm::mat4(1.0f);

int renderer::screen_resolution_x = 1280, renderer::screen_resolution_y = 720;

static std::vector<renderer::Model_Entity*> model_list;

unsigned int sprite_shader;
unsigned int cube_shader;

void renderer::init() {
	logger::info("STARTING RENDERER INITIALIZATION");
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_resolution_x, screen_resolution_y, SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE*/);
	context = SDL_GL_CreateContext(window);

	glEnable(GL_DEPTH_TEST);

	if (context != NULL) {
		glewExperimental = GL_TRUE;
		GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			logger::error("Error initializing GLEW! ", glewGetErrorString(glewError));
		}
	}

	projection = glm::perspective(glm::radians(45.0f), (float)screen_resolution_x / (float)screen_resolution_y, 0.1f, 100.0f);

	sprite_shader = create_shader("data/shaders/sprite_core.vs", "data/shaders/sprite_core.fs");
	cube_shader = create_shader("data/shaders/cube_core.vs", "data/shaders/cube_core.fs");

	logger::info("SUCCESFULY COMPLETED RENDERER INITIALIZATION");
}

void renderer::clean() {
	glDeleteProgram(sprite_shader);
	glDeleteProgram(cube_shader);

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void renderer::start_draw() {
	// Clear screen
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set aspect ratio
	glViewport(0, 0, screen_resolution_x, screen_resolution_y);
}

void renderer::draw_sprites() {
	for (auto& i : model_list) {
		i->draw();
	}
}

void renderer::stop_draw() {
	SDL_GL_SwapWindow(window);
}

renderer::Model_Entity::Model_Entity(std::string _model_path, glm::vec3* _position, glm::vec3 _colour)
	: model(_model_path), shader("data/shaders/model_loading.vs", "data/shaders/model_loading.fs"),
	position(_position), colour(_colour)
{
	glUseProgram(shader.id);
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "projection"), 1, false, glm::value_ptr(projection));

	model_list.push_back(this);
}

renderer::Model_Entity::~Model_Entity() {
	for (auto it = model_list.begin(); it != model_list.end();) {
		if (*it == this)
			it = model_list.erase(it);
		else
			++it;
	}
}

void renderer::Model_Entity::draw() {
	glUseProgram(shader.id);

	glm::mat4 new_model = glm::mat4(1.0f);
	new_model = glm::translate(new_model, *position);

	glUniform3fv(glGetUniformLocation(shader.id, "colour"), 1, glm::value_ptr(colour));
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "view"), 1, false, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "model"), 1, false, glm::value_ptr(new_model));

	model.draw(shader);
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

		glUniform3fv(glGetUniformLocation(square_shader, "u_color"), 1, glm::value_ptr(square->colour));

		glBindVertexArray(square->vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	for(auto& circle : circle_draw_list) {
		glUseProgram(circle_shader);

		glUniformMatrix4fv(glGetUniformLocation(circle_shader, "u_projection"), 1, false, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(circle_shader, "u_view"), 1, false, glm::value_ptr(view));

		glUniform3fv(glGetUniformLocation(circle_shader, "u_color"), 1, glm::value_ptr(circle->colour));

		glBindVertexArray(circle->vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	for(auto& line : line_draw_list) {
		glUseProgram(line_shader);

		glUniformMatrix4fv(glGetUniformLocation(line_shader, "u_projection"), 1, false, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(line_shader, "u_view"), 1, false, glm::value_ptr(view));

		glUniform3fv(glGetUniformLocation(line_shader, "u_color"), 1, glm::value_ptr(line->colour));

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
#endif // _DEBUG
