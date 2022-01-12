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

	void start_draw();
	void draw_sprites();
	void stop_draw();

	// TODO: Add Sprite Sheet
	// TODO: Batch Rendering

	struct Shader {
		unsigned int id;

		Shader(const char* vertexSource, const char* fragmentSource);
	private:
		void check_shader_compiler_errors(unsigned int shader);
	};

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 tex_coords;
	};

	struct Texture {
		unsigned int id;
		std::string type;
		std::string path;
	};

	inline unsigned int texture_from_file(const char* path, const std::string& directory, bool gamma = false) {
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
			logger::warning("Texture failed to load as path: ", path);
			stbi_image_free(data);
		}

		return textureID;
	}

	struct Mesh {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
		void draw(Shader& _shader);
	private:
		unsigned int vao, vbo, ebo;
		void setupMesh();
	};

	struct Model {
		std::vector<Texture> textures_loaded;
		std::vector<Mesh> meshes;
		std::string directory;
		bool gammaCorrection;

		Model(const std::string _path, bool gamma = false) : gammaCorrection(gamma)
		{
			load_model(_path);
		}

		void draw(Shader& _shader);

		void load_model(std::string _path);
		void process_node(aiNode* node, const aiScene* scene);
		Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> load_material_textures(aiMaterial* mat, aiTextureType type, std::string typeName);
	};

	struct Model_Entity {
		Model model;
		Shader shader;

		glm::vec3 colour;
		glm::vec3* position;

		Model_Entity(std::string _model_path, glm::vec3* _position, glm::vec3 _colour);
		~Model_Entity();

		void draw();
	};

#ifdef _DEBUG
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
#endif // _DEBUG
}
