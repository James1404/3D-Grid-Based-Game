#pragma once
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/GLU.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"

#include "log.h"

namespace colour {
	static const glm::vec3 black = glm::vec3(0, 0, 0);
	static const glm::vec3 white = glm::vec3(1, 1, 1);
	static const glm::vec3 grey = glm::vec3(0.5f, 0.5f, 0.5f);
	static const glm::vec3 red = glm::vec3(1, 0, 0);
	static const glm::vec3 green = glm::vec3(0, 1, 0);
	static const glm::vec3 blue = glm::vec3(0, 0, 1);
	static const glm::vec3 yellow = glm::vec3(1, 1, 0);
	static const glm::vec3 cyan = glm::vec3(0, 1, 1);
	static const glm::vec3 purple = glm::vec3(1, 0, 1);
	static const glm::vec3 pink = glm::vec3(1, 0.75f, 0.75f);
};

namespace renderer {
	extern SDL_Window* window;
	extern SDL_GLContext context;

	extern glm::mat4 projection;
	extern glm::mat4 view;

	extern int screen_resolution_x, screen_resolution_y;

	void init();
	void clean();

	void start_drawing_frame();
	void draw_models();
	void stop_drawing_frame();

	// TODO: Add Sprite Sheet
	// TODO: Batch Rendering

	struct shader_t {
		unsigned int id;
		std::string path;
	};

	static std::vector<shader_t> shaders_loaded;

	struct vertex_t {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 tex_coords;
	};

	struct texture_t {
		unsigned int id;
		std::string type;
		std::string path;
	};

	struct mesh_t {
		std::vector<vertex_t> vertices;
		std::vector<unsigned int> indices;
		std::vector<texture_t> textures;

		mesh_t(std::vector<vertex_t> vertices, std::vector<unsigned int> indices, std::vector<texture_t> textures);
		void draw(shader_t& _shader);
	private:
		unsigned int vao, vbo, ebo;
		void setupMesh();
	};

	struct model_t {
		std::vector<texture_t> textures_loaded;
		std::vector<mesh_t> meshes;
		std::string directory;
		std::string path;

		void draw(shader_t& _shader);

		void load_model(std::string _path);
		void process_node(aiNode* node, const aiScene* scene);
		mesh_t process_mesh(aiMesh* mesh, const aiScene* scene);
		std::vector<texture_t> load_material_textures(aiMaterial* mat, aiTextureType type, std::string typeName);
	};

	static std::vector<model_t> models_loaded;

	inline unsigned int texture_from_file(const char* path, const std::string& directory) {
		std::string filename = std::string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			log_warning("Texture failed to load as path: ", path);
			stbi_image_free(data);
		}

		return textureID;
	}

	inline shader_t shader_from_file(const char* shader_source) {
		shader_t _shader;
		bool skip = false;
		for (unsigned int i = 0; i < shaders_loaded.size(); i++) {
			if (shaders_loaded[i].path == shader_source) {
				_shader = shaders_loaded[i];
				skip = true;
			}
		}

		if (!skip) {
			std::string vShaderCode, fShaderCode;
			std::string shader_version;
			std::ifstream ifs(shader_source);
			if (ifs.is_open()) {
				enum class line_type_ {
					none = 0,
					vertex,
					fragment,
					version
				} line_type = line_type_::none;

				std::string line;
				while (std::getline(ifs, line)) {
					if (line == "[version]") {
						line_type = line_type_::version;
						continue;
					}
					else if (line == "[vertex]") {
						line_type = line_type_::vertex;
						continue;
					}
					else if (line == "[fragment]") {
						line_type = line_type_::fragment;
						continue;
					}

					if (line_type == line_type_::vertex) {
						//vStream << line;
						vShaderCode += line;
						vShaderCode += "\n";
					}
					else if (line_type == line_type_::fragment) {
						//fStream >> line;
						fShaderCode += line;
						fShaderCode += "\n";
					}
					else if (line_type == line_type_::version) {
						shader_version += line;
						shader_version += "\n";
					}
				}

				ifs.close();
			}
			else {
				log_error("COULD NOT OPEN SHADER FILE: ", shader_source);
			}

			vShaderCode.insert(0, shader_version);
			fShaderCode.insert(0, shader_version);

			const char* vertexShader = vShaderCode.c_str();
			const char* fragmentShader = fShaderCode.c_str();

			unsigned int vertex, fragment;

			//create and compiler shaders from its source
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vertexShader, NULL);
			glCompileShader(vertex);

			int success;
			char infoLog[512];
			glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(vertex, 512, NULL, infoLog);
				log_error("SHADER COMPILATION FAILED ", infoLog);
			}

			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fragmentShader, NULL);
			glCompileShader(fragment);

			glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				log_error("SHADER COMPILATION FAILED ", infoLog);
			}

			// create program and attach shaders
			unsigned int shader_program;
			shader_program = glCreateProgram();
			glAttachShader(shader_program, vertex);
			glAttachShader(shader_program, fragment);
			glLinkProgram(shader_program);

			// delete shaders as they are no longer needed
			glDeleteShader(vertex);
			glDeleteShader(fragment);

			_shader.id = shader_program;
			_shader.path = shader_source;
			shaders_loaded.push_back(_shader);

			log_info("LOADED AND COMPILED SHADER ", shader_source);
		}

		return _shader;
	}

	inline model_t model_from_file(const std::string _path) {
		for (unsigned int i = 0; i < models_loaded.size(); i++) {
			if (models_loaded[i].path == _path) {
				return models_loaded[i];
			}
		}

		model_t _model;
		_model.path = _path;

		_model.load_model(_model.path);
		models_loaded.push_back(_model);

		log_info("LOADED MODEL ", _path);

		return _model;
	}

	struct model_entity_t {
		model_t model;
		shader_t shader;

		glm::vec3* position;
		glm::vec3 rotation;
		glm::vec3 scale;

		bool is_paused;

		model_entity_t(std::string _model_path, glm::vec3* _position);
		~model_entity_t();

		void draw();
	};

	namespace debug {
		struct debug_drawing {
			unsigned int vao, vbo, ebo;

			glm::vec3 colour;

			debug_drawing();
			~debug_drawing();
		};

		void init_debug();
		void clean_debug();

		void clear_debug_list();
		void draw_debug();

		void draw_line(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 colour);
		void draw_box_wireframe(const glm::vec3 pos, const glm::vec3 size, const glm::vec3 colour);
	}
}
