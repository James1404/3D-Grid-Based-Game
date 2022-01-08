#version 330 core
in vec2 TexCoords;
out vec4 color;

// uniform sampler2D image;
uniform vec3 colour;

void main()
{
	// color = texture(image, TexCoords);
	color = vec4(colour, 1.0f);
}