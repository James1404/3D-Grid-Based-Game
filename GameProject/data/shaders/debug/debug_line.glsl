[version]
#version 330 core

[vertex]
layout (location = 0) in vec3 vertex;

uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
	gl_Position = u_projection * u_view * vec4(vertex, 1.0);
}

[fragment]
out vec4 o_color;

uniform vec3 u_color;

void main() {
	o_color = vec4(u_color, 1.0f);
}
