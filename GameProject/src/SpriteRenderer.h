#pragma once
#include <glm.hpp>
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class SpriteRenderer {
public:
	unsigned int ID;

	void InitSprite();
	void DrawSprite(glm::vec2 position);
private:
	void CheckCompileErrors(unsigned int shader);

	unsigned int quadVAO, quadVBO;
};