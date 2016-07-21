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

	worldPos = (ModelMatrix*vertex).xyz;
	normalVec = normalize(mat3(NormalMatrix)*vertexNormal);
	texCoord = vertexTexture;
	

	//for scan conversion
	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vertex;
}
