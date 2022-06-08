[version]
#version 330 core

[vertex]
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in mat4 transform;
// 4
// 5
// 6 RESERVED FOR TRANSFORM MATRIX
layout (location = 7) in int entity_index;

out vec2 o_tex_coords;
out vec3 o_normal;
out int o_entity_index;

uniform mat4 view_projection;

void main()
{
	o_tex_coords = texCoords;
	o_normal = normal;
	o_entity_index = entity_index;
	gl_Position = view_projection * transform * vec4(pos, 1.0);
}

[fragment]
layout(location = 0) out vec4 color1;
layout(location = 1) out int color2;

in vec2 o_tex_coords;
in vec3 o_normal;
flat in int o_entity_index;

uniform sampler2D Texture;

void main()
{
	vec3 light_dir = vec3(45.0, 5.0, 0.0);
	light_dir = normalize(-light_dir);

	vec3 diffuse = vec3(max(dot(normalize(o_normal), light_dir), .5));

	color1 = texture(Texture, o_tex_coords) * vec4(diffuse, 0);
	color2 = o_entity_index;
}
