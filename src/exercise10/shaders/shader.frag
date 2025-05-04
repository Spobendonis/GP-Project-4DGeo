#version 330 core

in vec3 Normal;

out vec4 FragColor;

uniform vec3 Color;

void main()
{
	float lighting = max(dot(normalize(Normal), normalize(vec3(1, 1, -1))), 0.3);
	FragColor = vec4(Color * lighting, 1);
}
