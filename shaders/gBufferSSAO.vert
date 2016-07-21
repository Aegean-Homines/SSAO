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
out vec3 viewPosition;

void main()
{
	mat4 MVMatrix = ViewMatrix * ModelMatrix;
	mat4 MVPMatrix = ProjectionMatrix * MVMatrix;
	viewPosition = (MVMatrix * vertex).xyz;
	gl_Position = MVPMatrix * vertex;
}