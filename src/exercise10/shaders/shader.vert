#version 330 core

layout (location = 0) in vec4 VertexPosition;
layout (location = 1) in vec4 VertexNormal;

out vec3 Normal;
out float VertexW;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

void main()
{
	//VertexNormal has a 4th dimension, which is currently being thrown away
	VertexW = (WorldMatrix * VertexPosition).w;
	Normal = normalize((WorldMatrix * VertexNormal).xyz);
	gl_Position = ViewProjMatrix * vec4((WorldMatrix * VertexW * VertexPosition).xyz, 1.0);
} 
