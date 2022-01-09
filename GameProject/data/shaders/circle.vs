#version 330 core
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 texcoords;

out vec2 TexCoords;

uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
	TexCoords = texcoords;
	
	gl_Position = u_projection * u_view * vec4(vertex, 0.0, 1.0);
}