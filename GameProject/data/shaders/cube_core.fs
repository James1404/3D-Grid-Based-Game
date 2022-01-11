#version 330 core
in vec2 TexCoords;
out vec4 color;

// uniform sampler2D image;
uniform vec3 colour;

float near = 0.1;
float far = 100.0;

float linearize_depth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near)) / far;
}

void main()
{
	float depth = linearize_depth(gl_FragCoord.z);
	// color = texture(image, TexCoords);
	color = vec4(colour, 1.0f);
}