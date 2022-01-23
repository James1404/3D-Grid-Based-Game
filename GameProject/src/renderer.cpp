#include "renderer.h"

#include <memory>
#include <map>

//
// ----- MESH -----
//

renderer::mesh_t::mesh_t(std::vector<vertex_t> vertices, std::vector<unsigned int> indices, std::vector<texture_t> textures) {
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	setupMesh();
}

void renderer::mesh_t::draw(shader_t& _shader) {
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

void renderer::mesh_t::setupMesh() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex_t), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, tex_coords));

	glBindVertexArray(0);
}

//
// ----- MODEL -----
//


void renderer::model_t::draw(shader_t& _shader) {
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].draw(_shader);
	}
}

void renderer::model_t::load_model(std::string _path) {
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(_path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		log_error("ERROR::ASSIMP::", import.GetErrorString());
		return;
	}
	directory = _path.substr(0, _path.find_last_of('/'));

	process_node(scene->mRootNode, scene);
}

void renderer::model_t::process_node(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(process_mesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		process_node(node->mChildren[i], scene);
	}
}

renderer::mesh_t renderer::model_t::process_mesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<vertex_t> vertices;
	std::vector<unsigned int> indices;
	std::vector<texture_t> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		vertex_t vertex;

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
		std::vector<texture_t> diffuseMaps = load_material_textures(material, aiTextureType_DIFFUSE, " texture_diffuse");

		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<texture_t> specularMaps = load_material_textures(material, aiTextureType_SPECULAR, " texture_specular");

		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return mesh_t(vertices, indices, textures);
}

std::vector<renderer::texture_t> renderer::model_t::load_material_textures(aiMaterial* mat, aiTextureType type, std::string typeName) {
	std::vector<texture_t> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++) {
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip) {
			texture_t texture;
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
// ----- RENDERER -----
//

SDL_Window* renderer::window;
SDL_GLContext renderer::context;

glm::mat4 renderer::projection;
glm::mat4 renderer::view = glm::mat4(1.0f);

int renderer::screen_resolution_x = 1280, renderer::screen_resolution_y = 720;

static std::vector<renderer::model_entity_t*> model_list;

void renderer::init() {
	log_info("STARTING RENDERER INITIALIZATION");
	
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
			log_error("Error initializing GLEW! ", glewGetErrorString(glewError));
		}
	}

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

renderer::model_entity_t::model_entity_t(std::string _model_path, glm::vec3* _position)
	: model(model_from_file(_model_path)), shader(shader_from_file("data/shaders/model_loading.shader")),
	position(_position), is_paused(false), rotation(0,0,0), scale(1,1,1)
{
	glUseProgram(shader.id);
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "projection"), 1, false, glm::value_ptr(projection));

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

	glUseProgram(shader.id);

	glm::mat4 new_model = glm::mat4(1.0f);
	new_model = glm::translate(new_model, *position);
	//new_model = glm::rotate(new_model, rotation);
	new_model = glm::rotate(new_model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	new_model = glm::rotate(new_model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	new_model = glm::rotate(new_model, glm::radians(rotation.z), glm::vec3(0, 0, 1));

	new_model = glm::scale(new_model, scale);

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

renderer::shader_t circle_shader;
renderer::shader_t line_shader;
void renderer::debug::init_debug() {
	circle_shader = shader_from_file("data/shaders/debug/debug_circle.shader");
	line_shader	= shader_from_file("data/shaders/debug/debug_line.shader");
}

void renderer::debug::clean_debug() {
	clear_debug_list();

	glDeleteProgram(circle_shader.id);
	glDeleteProgram(line_shader.id);
}

std::vector<std::shared_ptr<renderer::debug::debug_drawing>> circle_draw_list;
std::vector<std::shared_ptr<renderer::debug::debug_drawing>> line_draw_list;

void renderer::debug::clear_debug_list () {
	circle_draw_list.clear();
	line_draw_list.clear();
}

void renderer::debug::draw_debug() {
	for(auto& circle : circle_draw_list) {
		glUseProgram(circle_shader.id);

		glUniformMatrix4fv(glGetUniformLocation(circle_shader.id, "u_projection"), 1, false, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(circle_shader.id, "u_view"), 1, false, glm::value_ptr(view));

		glUniform3fv(glGetUniformLocation(circle_shader.id, "u_color"), 1, glm::value_ptr(circle->colour));

		glBindVertexArray(circle->vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	for(auto& line : line_draw_list) {
		glUseProgram(line_shader.id);

		glUniformMatrix4fv(glGetUniformLocation(line_shader.id, "u_projection"), 1, false, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(line_shader.id, "u_view"), 1, false, glm::value_ptr(view));

		glUniform3fv(glGetUniformLocation(line_shader.id, "u_color"), 1, glm::value_ptr(line->colour));

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
