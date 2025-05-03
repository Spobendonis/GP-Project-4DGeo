#version 330 core

in vec3 Normal;

out vec4 FragColor;

uniform vec3 Color;

void main()
{
	FragColor = vec4(Color, 1);
}
