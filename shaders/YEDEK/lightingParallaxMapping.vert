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

uniform vec3 lightPos;

//out
out vec3 worldPos;
out vec3 normalVec; //N
out vec3 originalLightVec; //L
out vec3 originalEyeVec; //V
out vec2 texCoord;

out mat3 TBN;

out vec3 tangentLightPos;
out vec3 tangentViewPos;
out vec3 tangentWorldPos;

void main()
{

	vec3 viewPos = (ViewInverse*vec4(0,0,0,1)).xyz;
	//vertex normal for lighting calculation (N)
	normalVec = normalize(mat3(NormalMatrix)*vertexNormal);
	worldPos = (ModelMatrix*vertex).xyz;
	
	originalLightVec = lightPos - worldPos;
	originalEyeVec = viewPos - worldPos;
	texCoord = vertexTexture;

	// For normal calculations from normal map
	vec3 T = normalize(mat3(ModelMatrix) * vertexTangent);
    vec3 N = normalize(mat3(ModelMatrix) * vertexNormal);  
	vec3 B = cross(vertexTangent, vertexNormal);
	TBN = transpose(mat3(T, B, N));

	tangentLightPos = TBN * lightPos;
	tangentViewPos = TBN * viewPos;
	tangentWorldPos = TBN * worldPos;

	//for scan conversion
	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vertex;
}
