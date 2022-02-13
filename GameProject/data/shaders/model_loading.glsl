[version]
#version 330 core

[vertex]
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;

uniform mat4 mvp;

void main()
{
	TexCoords = aTexCoords;
	//gl_Position = projection * view * model * vec4(aPos, 1.0);
	gl_Position = mvp * vec4(aPos, 1.0);
}

[fragment]
layout(location = 0) out vec4 color1;
layout(location = 1) out int color2;

in vec2 TexCoords;

uniform sampler2D Texture;
uniform int entity_index;

void main()
{
	color1 = texture(Texture, TexCoords);
	color2 = entity_index;
}
