[version]
#version 330 core

[vertex]
layout (location = 0) in vec3 pos;

uniform mat4 u_view_projection;

void main() {
	gl_Position = u_view_projection * vec4(pos, 1.0);
}

[fragment]
out vec4 o_color;

uniform vec3 u_color;

void main() {
	o_color = vec4(u_color, 1.0f);
}
