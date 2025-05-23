#version 330 core

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;

uniform float AmbientReflection;
uniform float DiffuseReflection;
uniform float SpecularReflection;
uniform float SpecularExponent;

uniform vec3 AmbientColor;
uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform vec3 CameraPosition;

uniform sampler2D Texture;
uniform bool UsingTexture;

//Blinn-Phong stuff
vec3 GetAmbientReflection(vec3 objectColor)
{
	return AmbientColor * AmbientReflection * objectColor;
}

vec3 GetDiffuseReflection(vec3 objectColor, vec3 lightVector, vec3 normalVector)
{
	return LightColor * DiffuseReflection * objectColor * max(dot(lightVector, normalVector), 0.0f);
}

vec3 GetSpecularReflection(vec3 lightVector, vec3 viewVector, vec3 normalVector)
{
	vec3 halfVector = normalize(lightVector + viewVector);
	return LightColor * SpecularReflection * pow(max(dot(halfVector, normalVector), 0.0f), SpecularExponent);
}

vec3 GetBlinnPhongReflection(vec3 objectColor, vec3 lightVector, vec3 viewVector, vec3 normalVector)
{
	return GetAmbientReflection(objectColor)
		 + GetDiffuseReflection(objectColor, lightVector, normalVector)
		 + GetSpecularReflection(lightVector, viewVector, normalVector);
}

void main()
{
	// Reuses the code from the Exercise where we implement Blinn-Phong
	vec4 objectColor = Color;
	vec3 lightVector = normalize(LightPosition - Position);
	vec3 viewVector = normalize(CameraPosition - Position);
	vec3 normalVector = normalize(Normal);

	vec4 tColor = vec4(1.0f);
	if (UsingTexture) {
		tColor = texture(Texture, TexCoord);
	}
	FragColor = vec4(GetBlinnPhongReflection(objectColor.rgb * tColor.rgb, lightVector, viewVector, normalVector), 1.0f);
}
