#version 330 core

layout (location = 0) in vec4 VertexPosition;
layout (location = 1) in vec4 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 Position;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 WorldRotationMatrix;
uniform vec4 WorldTranslationVector;
uniform vec4 WorldScaleVector;
uniform mat4 ViewProjMatrix;

void main()
{
	// Gets the real 4D location of the Vertex after the transformations
	vec4 Real4DPosition = WorldTranslationVector + (WorldRotationMatrix * (WorldScaleVector * VertexPosition));
	
	float VertexW = Real4DPosition.w;

	// Calculate the 3D Projected position of the Vertex, by treating The Vertex's w as a scale factor.
	// Since there is a non-uniform scale (vertex scale depends on VertexW and WorldScaleVector), the ignores the scale
	Position = (WorldScaleVector * ((WorldTranslationVector / WorldScaleVector) + (WorldRotationMatrix * (VertexW * VertexPosition)))).xyz;

	TexCoord = VertexTexCoord;

	Normal = normalize((WorldRotationMatrix * VertexNormal).xyz);
	gl_Position = ViewProjMatrix * vec4(Position, 1.0f);
} 
