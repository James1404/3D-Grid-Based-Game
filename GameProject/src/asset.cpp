#include "asset.h"

#include "log.h"

//
// transform
//

transform_t::transform_t()
	: position(glm::vec3(0)), rotation(glm::vec3(0)), scale(glm::vec3(1)) {}

transform_t::transform_t(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
	: position(position), rotation(rotation), scale(scale) {}

glm::mat4 transform_t::get_matrix()
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);

	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));

	model = glm::scale(model, scale);

	return model;
}

//
// MODEL
//

void model_t::load_model(std::string _path)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(_path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_PreTransformVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		log_error("ERROR::ASSIMP::", import.GetErrorString());
		return;
	}
	directory = _path.substr(0, _path.find_last_of('/'));

	process_node(scene->mRootNode, scene);
	setup_buffers();
}

void model_t::process_node(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		process_mesh(mesh, scene);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		process_node(node->mChildren[i], scene);
	}
}

void model_t::process_mesh(aiMesh* mesh, const aiScene* scene)
{
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
		{
			vertex.tex_coords = glm::vec2(0);
		}

		vertices.push_back(vertex);
	}

	int indices_count = indices.size();
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j] + indices_count);
		}
	}
}

void model_t::setup_buffers()
{
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
// ASSET MANAGER
//

std::shared_ptr<shader_t> asset_manager_t::load_shader_from_file(std::string path)
{
	std::shared_ptr<shader_t> result;

	auto iter = shaders.find(path);
	if(iter != shaders.end())
	{
		result = iter->second;
	}
	else
	{
		std::string vShaderCode, fShaderCode;
		std::string shader_version;
		std::ifstream ifs(path);
		if (ifs.is_open())
		{
			enum class line_type_
			{
				none = 0,
				vertex,
				fragment,
				version
			} line_type = line_type_::none;

			std::string line;
			while (std::getline(ifs, line))
			{
				if (line == "[version]")
				{
					line_type = line_type_::version;
					continue;
				}
				else if (line == "[vertex]")
				{
					line_type = line_type_::vertex;
					continue;
				}
				else if (line == "[fragment]")
				{
					line_type = line_type_::fragment;
					continue;
				}

				if (line_type == line_type_::vertex)
				{
					vShaderCode += line;
					vShaderCode += "\n";
				}
				else if (line_type == line_type_::fragment)
				{
					fShaderCode += line;
					fShaderCode += "\n";
				}
				else if (line_type == line_type_::version)
				{
					shader_version += line;
					shader_version += "\n";
				}
			}

			ifs.close();
		}
		else
		{
			log_error("Could Not Open Shader File: ", path);
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
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			log_error("Shader Compilation Failed ", infoLog);
		}

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fragmentShader, NULL);
		glCompileShader(fragment);

		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			log_error("Shader Compilation Failed ", infoLog);
		}

		// create program and attach shaders
		unsigned int shader_program;
		shader_program = glCreateProgram();

		glAttachShader(shader_program, vertex);
		glAttachShader(shader_program, fragment);
		glLinkProgram(shader_program);

		// delete shaders as they are no longer needed
		glDetachShader(shader_program, vertex);
		glDetachShader(shader_program, fragment);

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		result = std::make_shared<shader_t>();
		result->id = shader_program;

		shaders.emplace(path, result); 

		log_info("Loaded And Compiled Shader", path);
	}

	return result;
}

std::shared_ptr<texture_t> asset_manager_t::load_texture_from_file(std::string path)
{
	std::shared_ptr<texture_t> result;

	auto iter = textures.find(path);
	if(iter != textures.end())
	{
		result = iter->second;
	}
	else 
	{
		result = std::make_shared<texture_t>();
		glGenTextures(1, &result->id);

		int width, height, nrcomponents;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrcomponents, 0);
		if (data)
		{
			GLenum format;
			if (nrcomponents == 1)
				format = GL_RED;
			else if (nrcomponents == 3)
				format = GL_RGB;
			else if (nrcomponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, result->id);
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
			log_warning("Texture Failed To Load As Path: ", path);
			stbi_image_free(data);
		}

		textures.emplace(path, result); 

		log_info("Loaded Texture", path);
	}

	return result;
}

std::shared_ptr<model_t> asset_manager_t::load_model_from_file(std::string path)
{
	std::shared_ptr<model_t> result;

	auto iter = models.find(path);
	if(iter != models.end())
	{
		result = iter->second;
	}
	else
	{
		result = std::make_shared<model_t>();
		result->load_model(path);
		models.emplace(path, result); 

		log_info("Loaded Model ", path);
	}

	return result;
}

void asset_manager_t::init()
{
}

void asset_manager_t::shutdown()
{
	shaders.clear();
	textures.clear();
	models.clear();
}
