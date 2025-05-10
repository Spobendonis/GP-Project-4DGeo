#version 330 core

in vec3 Normal;
in float VertexW;

out vec4 FragColor;

uniform vec3 Color;

void main()
{
	float lighting = max(dot(Normal, normalize(vec3(1, 1, -1))), 0.5);
	//float lighting = max(1/VertexW, 0.2);
	//FragColor = vec4(VertexW, 1);
	FragColor = vec4(Color * lighting, 1);
}
