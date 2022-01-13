#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 colour;

void main()
{
	FragColor = texture(texture_diffuse1, TexCoords);
	//FragColor = vec4(colour, 1);
}