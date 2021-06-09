#include "SpriteRenderer.h"

#include "Game.h"
#include "ResourceManager.h"

#include <GL/glew.h>
#include "stb_image.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

void SpriteRenderer::InitSprite() {
	// Load and Generate Textures Shaders
	std::string vShaderCode, fShaderCode;
	std::ifstream vShaderFile, fShaderFile;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vShaderFile.open("resources/shaders/core.vs");
		fShaderFile.open("resources/shaders/core.fs");
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

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexShader, NULL);
	glCompileShader(vertex);

	int success;
	char infoLog[512];
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED %.4s\n", infoLog);
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentShader, NULL);
	glCompileShader(fragment);

	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED %.4s\n", infoLog);
	}

	this->ID = glCreateProgram();
	glAttachShader(this->ID, vertex);
	glAttachShader(this->ID, fragment);
	glLinkProgram(this->ID);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	texture = ResourceManager::LoadTexture("resources/textures/awesomeface.png");

	unsigned int VBO, EBO;
	float vertices[] = {
		// positions	// texture coords
		 1.0f,  1.0f,	1.0f, 1.0f, // top right
		 1.0f,  0.0f,	1.0f, 0.0f, // bottom right
		 0.0f,  0.0f,	0.0f, 0.0f, // bottom left
		 0.0f,  1.0f,	0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	glGenVertexArrays(1, &this->quadVAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(this->quadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glUniform1i(glGetUniformLocation(this->ID, "image"), 0);

	glUseProgram(this->ID);
	glUniformMatrix4fv(glGetUniformLocation(this->ID, "projection"), 1, GL_FALSE, glm::value_ptr(Game::projection));
}

void SpriteRenderer::DrawSprite(glm::vec2 position) {
	glUseProgram(this->ID);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0f));
	model = glm::scale(model, glm::vec3(100.0f, 100.0f, 1.0f));

	glUniformMatrix4fv(glGetUniformLocation(this->ID, "view"), 1, GL_FALSE, glm::value_ptr(Game::view));
	glUniformMatrix4fv(glGetUniformLocation(this->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(this->quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void SpriteRenderer::CheckCompileErrors(unsigned int shader) {
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("ERROR::SHADER::COMPILATION_FAILED%.4s\n", infoLog);
	}
}