/////////////////////////////////////////////////////////////////////////
// Vertex shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

in vec4 vertex;

uniform mat4 ModelMatrix;
uniform mat4 LightViewMatrix;
uniform mat4 LightProjectionMatrix;

out vec4 position;

void main()
{
	gl_Position = LightProjectionMatrix * LightViewMatrix * ModelMatrix * vertex;
	position = gl_Position;
}
