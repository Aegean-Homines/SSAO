/////////////////////////////////////////////////////////////////////////
// Vertex shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

//in
//coming from models object
in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;


//uniform
uniform mat4 ModelMatrix;
uniform mat4 NormalMatrix;
uniform vec3 lightPos;
uniform int reflectionDirection;
uniform mat4 ShadowMatrix;

//out
out vec3 normalVec; //N
out vec3 lightVec; //L
out vec3 eyeVec; //V
out vec2 texCoord;
out vec4 shadowCoord;

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

	//assigning output values
	normalVec = normalize(mat3(NormalMatrix)*vertexNormal);
	vec3 vertexPosition = (ModelMatrix*vertex).xyz;
	lightVec = lightPos - vertexPosition;
	eyeVec = -vertexPosition;
	texCoord = vertexTexture;
	
	//Calculation of reflection
	vec3 R = vertexPosition;
	float lengthOfR = length(R);
	vec3 normalizedR = R / lengthOfR; //Normalize

	//for shadow
	shadowCoord = ShadowMatrix * ModelMatrix * vertex;

	float d;
	if(reflectionDirection == 0) //up
	{
		d = 1 - normalizedR.z;
		gl_Position = vec4(normalizedR.x / d, normalizedR.y / d, (-normalizedR.z * lengthOfR / 100) - 0.9, 1);
	}
	else						//down
	{
		d = 1 + normalizedR.z;
		gl_Position = vec4(normalizedR.x / d, normalizedR.y / d, (normalizedR.z * lengthOfR / 100) - 0.9, 1);
	}

}
