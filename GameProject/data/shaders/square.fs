#version 330 core

out vec4 o_color;

uniform vec3 u_color;

void main() {
	o_color = vec4(u_color, 1.0f);
}