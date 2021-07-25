#pragma once
#include <glm.hpp>

#include "Shader.h"

// TODO: Add Sprite Sheet
// TODO: Batch Rendering

class SpriteRenderer {
public:
	void InitSprite(const char* path);
	void DrawSprite(glm::vec2 position);

	int width, height;
private:
	Shader shader;

	unsigned int quadVAO;
	unsigned int texture;
};