#version 330 core
layout (location = 0) in vec2 vertex; // postion
layout (location = 1) in vec2 texcoords; // texCoords

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	TexCoords = texcoords;
	gl_Position = projection * view * model * vec4(vertex, 0.0, 1.0);
}