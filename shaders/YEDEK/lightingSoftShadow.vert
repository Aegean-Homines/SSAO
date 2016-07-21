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
uniform mat4 ViewMatrix, ViewInverse;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform mat4 ShadowMatrix;

uniform vec3 lightPos;

//out
out vec3 worldPos;
out vec3 normalVec; //N
out vec3 lightVec; //L
out vec3 eyeVec; //V
out vec2 texCoord;
out vec4 shadowCoord;

void main()
{

	//vertex normal for lighting calculation (N)
	normalVec = normalize(mat3(NormalMatrix)*vertexNormal);
	worldPos = (ModelMatrix*vertex).xyz;
	
	lightVec = lightPos - worldPos;
	eyeVec = (ViewInverse*vec4(0,0,0,1)).xyz - worldPos;
	texCoord = vertexTexture;
	
	//for shadow - to be used in fragment shader
	shadowCoord = ShadowMatrix * ModelMatrix * vertex;

	//for scan conversion
	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vertex;
	//gl_Position = vertex;
}
