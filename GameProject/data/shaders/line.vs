#version 330 core
layout (location = 0) in vec3 vertex;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform bool u_is_screen_space;

void main() {
	gl_Position = u_projection * u_view * vec4(vertex, 1.0);
}