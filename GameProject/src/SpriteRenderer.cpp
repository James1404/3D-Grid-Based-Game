#include "SpriteRenderer.h"

#include "Game.h"
#include "ResourceManager.h"

#include <GL/glew.h>
#include "stb_image.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

void SpriteRenderer::InitSprite(const char* path) {
	shader.Compile("resources/shaders/core.vs", "resources/shaders/core.fs");

	texture = ResourceManager::LoadTexture(path, this->width, this->height);

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

	shader.SetInteger("image", 0);
	shader.SetMatrix4("projection", Game::projection, true);
}

void SpriteRenderer::DrawSprite(glm::vec2 position) {
	shader.Use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0f));
	model = glm::scale(model, glm::vec3(width, height, 1.0f));

	shader.SetMatrix4("view", Game::view);
	shader.SetMatrix4("model", model);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->texture);

	glBindVertexArray(this->quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}