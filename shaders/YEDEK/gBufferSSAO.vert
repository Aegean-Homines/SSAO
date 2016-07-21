/////////////////////////////////////////////////////////////////////////
// Vertex shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

//in
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexture;
layout (location = 3) in vec3 vertexTangent;

//uniform
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

//out
out vec3 worldPos;
out vec3 normalVec;
out vec2 texCoord;

void main()
{
	vec4 viewPosition = ViewMatrix * ModelMatrix * vertex;
	worldPos = viewPosition.xyz;
	normalVec = normalize(mat3(NormalMatrix * ViewMatrix)*vertexNormal);
	texCoord = vertexTexture;

	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vertex;
}