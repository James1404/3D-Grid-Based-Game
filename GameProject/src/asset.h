#pragma once
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>

#include <GL/glew.h>
#include <GL/GLU.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"

#include "log.h"

struct transform_t
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	transform_t();
	transform_t(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

	glm::mat4 get_matrix();
};

struct shader_t
{
	unsigned int id;

	~shader_t()
	{
		glDeleteProgram(id);
	}
};

struct texture_t
{
	unsigned int id;

	~texture_t()
	{
		glDeleteTextures(1, &id);
	}
};

struct vertex_t
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
};

/*
struct mesh_t
{
	std::vector<vertex_t> vertices;
	std::vector<unsigned int> indices;

	mesh_t(std::vector<vertex_t> vertices, std::vector<unsigned int> indices);
	~mesh_t();

	void draw(shader_t& _shader);
private:
	unsigned int vao, vbo, ebo;
	void setupMesh();
};
*/

struct model_t
{
	std::vector<vertex_t> vertices;
	std::vector<unsigned int> indices;

	unsigned int vao, vbo, ebo;

	//int mesh_count;
	std::string directory;

	void draw(shader_t& _shader);

	void load_model(std::string _path);
	void process_node(aiNode* node, const aiScene* scene);
	void process_mesh(aiMesh* mesh, const aiScene* scene);
	void setup_buffers();
};

struct asset_manager_t
{
	std::map<std::string, std::shared_ptr<shader_t>> shaders;	
	std::map<std::string, std::shared_ptr<texture_t>> textures;	
	std::map<std::string, std::shared_ptr<model_t>> models;	

	std::shared_ptr<shader_t> load_shader_from_file(std::string path);
	std::shared_ptr<texture_t> load_texture_from_file(std::string path);
	std::shared_ptr<model_t> load_model_from_file(std::string path);

	void init();
	void shutdown();

	static asset_manager_t& get()
	{
		static asset_manager_t* instance = NULL;
		if(instance == NULL)
		{
			instance = new asset_manager_t;
		}
		assert(instance);
		return *instance;
	}
};
