#version 330 core

layout (location = 0) in vec4 VertexPosition;
layout (location = 1) in vec4 VertexNormal;

out vec3 Position;
out vec3 Normal;

uniform mat4 WorldRotationMatrix;
uniform vec4 WorldTranslationVector;
uniform vec4 WorldScaleVector;
uniform mat4 ViewProjMatrix;

void main()
{
	//Get the projection position accounting for 4D
	vec4 Real4DPosition = WorldTranslationVector + (WorldRotationMatrix * (WorldScaleVector * VertexPosition));
	
	float VertexW = Real4DPosition.w;

	//Treat VertexW as a scale factor 
	Position = (WorldScaleVector * ((WorldTranslationVector / WorldScaleVector) + (WorldRotationMatrix * (VertexW * VertexPosition)))).xyz;

	Normal = normalize((WorldRotationMatrix * VertexNormal).xyz);
	gl_Position = ViewProjMatrix * vec4(Position, 1.0f);
} 
