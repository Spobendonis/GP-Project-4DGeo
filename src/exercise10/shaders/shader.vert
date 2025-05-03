#version 330 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

void main()
{
	gl_Position = ViewProjMatrix * WorldMatrix * vec4(VertexPosition, 1.0);
}
