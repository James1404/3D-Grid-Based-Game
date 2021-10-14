#version 330 core
layout (location = 0) in vec2 vertex;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform bool u_is_screen_space;

void main() {
	if(u_is_screen_space) {
		gl_Position = u_projection * vec4(vertex, 0.0, 1.0);
	} else {
		gl_Position = u_projection * u_view * vec4(vertex, 0.0, 1.0);
	}
}