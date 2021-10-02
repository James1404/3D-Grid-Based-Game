#version 330 core
layout (location = 0) in vec2 i_pos;

uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
	gl_Position = u_projection * u_view * vec4(i_pos, 0.0, 1.0);
}