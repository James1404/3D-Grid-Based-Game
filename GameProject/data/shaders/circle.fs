#version 330 core
in vec2 TexCoords;
out vec4 o_color;

uniform vec3 u_color;

void main() {
	vec2 uv = TexCoords * 2.0f - 1.0f;

	o_color.x = uv.x;
	o_color.y = uv.y;
	o_color.z = 0.0f;
	
	float distance = 1.0f - sqrt(dot(uv, uv));
	distance = smoothstep(0.0f, 0.1f, distance);
	
	o_color = vec4(distance * u_color, distance);
}