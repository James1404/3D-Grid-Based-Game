#pragma once
#include <glm.hpp>

class Shader {
public:
	unsigned int ID;

	Shader() { }
	Shader& Use();

	void Compile(const char* vertexSource, const char* fragmentSource);

    void SetFloat(const char* name, float value, bool useShader = false);
    void SetInteger(const char* name, int value, bool useShader = false);
    void SetVector2f(const char* name, const glm::vec2& value, bool useShader = false);
    void SetMatrix4(const char* name, const glm::mat4& matrix, bool useShader = false);
private:
	void CheckCompileErrors(unsigned int shader);
};