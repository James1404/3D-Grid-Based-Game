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
out int o_entity_index;

uniform mat4 view_projection;

void main()
{
	o_tex_coords = texCoords;
	o_entity_index = entity_index;
	gl_Position = view_projection * transform * vec4(pos, 1.0);
}

[fragment]
layout(location = 0) out vec4 color1;
layout(location = 1) out int color2;

in vec2 o_tex_coords;
flat in int o_entity_index;

uniform sampler2D Texture;

void main()
{
	color1 = texture(Texture, o_tex_coords);
	color2 = o_entity_index;
}
