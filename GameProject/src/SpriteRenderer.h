#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

class SpriteRenderer {
public:
	unsigned int ID;

	void InitSprite();
	void DrawSprite(glm::vec2 position, glm::vec2 scale);
private:
	void CheckCompileErrors(unsigned int shader);

	unsigned int quadVAO;
	unsigned int texture;
};