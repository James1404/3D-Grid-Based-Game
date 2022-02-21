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

out vec2 TexCoords;
out int EntityIndex;

uniform mat4 view_projection;

void main()
{
	TexCoords = texCoords;
	EntityIndex = entity_index;
	gl_Position = view_projection * transform * vec4(pos, 1.0);
}

[fragment]
layout(location = 0) out vec4 color1;
layout(location = 1) out int color2;

in vec2 TexCoords;
//in int EntityIndex;

uniform sampler2D Texture;

void main()
{
	color1 = texture(Texture, TexCoords);
	color2 = 0;
}
