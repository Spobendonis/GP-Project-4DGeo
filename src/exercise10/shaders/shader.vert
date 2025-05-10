#version 330 core

layout (location = 0) in vec4 VertexPosition;
layout (location = 1) in vec4 VertexNormal;

out vec3 Normal;
out float VertexW;

uniform mat4 WorldRotationMatrix;
uniform vec4 WorldTranslationVector;
uniform vec4 WorldScaleVector;
uniform mat4 ViewProjMatrix;

void main()
{
	//Get the project position accounting for 4D
	vec4 Real4DPosition = WorldTranslationVector + (WorldRotationMatrix * (WorldScaleVector * VertexPosition));
	VertexW = Real4DPosition.w;

	//Treat VertexW as a scale factor 
	vec4 TransformedPosition = WorldTranslationVector + (WorldRotationMatrix * (WorldScaleVector * VertexW * VertexPosition));

	Normal = normalize((WorldRotationMatrix * VertexNormal).xyz);
	gl_Position = ViewProjMatrix *
				vec4((TransformedPosition).xyz, 1.0);
} 
