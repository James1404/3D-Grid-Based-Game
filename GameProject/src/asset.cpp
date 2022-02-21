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

void model_t::add_instance(transform_t* transform, int entity_index)
{
	model_instance_data_t data;
	data.transform = transform;

#ifdef _DEBUG
	data.index = entity_index;
#endif // _DEBUG

	instance_data.push_back(data);
}

void model_t::construct_instance_buffers()
{
	instance_buffer_size = instance_data.size();

	glBindVertexArray(vao);

	glGenBuffers(1, &instance_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);

#ifdef _DEBUG
	glBufferData(GL_ARRAY_BUFFER, (sizeof(glm::mat4) + sizeof(int)) * instance_buffer_size, NULL, GL_DYNAMIC_DRAW);
#else
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * instance_buffer_size, NULL, GL_DYNAMIC_DRAW);
#endif

	std::vector<glm::mat4> transforms;
#ifdef _DEBUG
	std::vector<int> indexs;
#endif // _DEBUG
	for(int i = 0; i < instance_buffer_size; i++)
	{
		transforms.push_back(instance_data[i].transform->get_matrix());
#ifdef _DEBUG
		indexs.push_back(instance_data[i].index);
#endif // _DEBUG
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * transforms.size(), &transforms[0]);
#ifdef _DEBUG
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * transforms.size(), sizeof(int) * indexs.size(), &indexs[0]);
#endif // _DEBUG

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

#ifdef _DEBUG
	glEnableVertexAttribArray(7);
	glVertexAttribIPointer(7, 1, GL_INT, sizeof(int), (void*)(sizeof(glm::mat4) * transforms.size()));
	glVertexAttribDivisor(7, 1);
#endif // _DEBUG

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void model_t::update_instance_buffers()
{
	std::vector<glm::mat4> transforms;

	for(int i = 0; i < instance_buffer_size; i++)
	{
		transforms.push_back(instance_data[i].transform->get_matrix());
	}

	glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * transforms.size(), &transforms[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void model_t::draw()
{
	update_instance_buffers();

	glBindVertexArray(vao);
	glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, instance_buffer_size);
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
			log_error("COULD NOT OPEN SHADER FILE: ", path);
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
		glDetachShader(shader_program, vertex);
		glDetachShader(shader_program, fragment);

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		result = std::make_shared<shader_t>();
		result->id = shader_program;

		shaders.emplace(path, result); 

		log_info("LOADED AND COMPILED SHADER ", path);
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
			log_warning("texture failed to load as path: ", path);
			stbi_image_free(data);
		}

		textures.emplace(path, result); 
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

		log_info("LOADED MODEL ", path);
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
