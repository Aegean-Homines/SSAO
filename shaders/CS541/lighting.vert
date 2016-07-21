/////////////////////////////////////////////////////////////////////////
// Vertex shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

//in
in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

//uniform
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix, ViewInverse;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec3 eyePos;
uniform vec3 lightPos;

//out
out vec3 worldPos;
out vec3 normalVec; //N
out vec3 lightVec; //L
out vec3 eyeVec; //V
out vec2 texCoord;

out vec4 Vertex;
out vec3 VertexNormal;
out vec2 VertexTexture;
out vec3 VertexTangent;

void main()
{
	Vertex = vertex;
	VertexNormal =  vertexNormal;
	VertexTexture = vertexTexture; 
	VertexTangent = vertexTangent; 

	//vertex normal for lighting calculation (N)
	normalVec = normalize(mat3(NormalMatrix)*vertexNormal);

	worldPos = (ModelMatrix*vertex).xyz;
	
	lightVec = lightPos - worldPos;
	eyeVec = (ViewInverse*vec4(0,0,0,1)).xyz - worldPos;
	texCoord = vertexTexture;

	//for scan conversion
	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vertex;
}
