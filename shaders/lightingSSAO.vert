#version 330

//in
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexture;
layout (location = 3) in vec3 vertexTangent;

uniform mat4 ProjectionMatrix, ViewMatrix, ModelMatrix, NormalMatrix, ViewInverse;
uniform vec3 lightPos;

out vec3 lightVec; //L
out vec3 eyeVec; //V
out vec2 texCoord;
out vec3 normalVec;
out vec3 worldPos;

void main(){
	vec3 viewPos = (ViewInverse*vec4(0,0,0,1)).xyz;

	texCoord = vertexTexture;
	normalVec = (ModelMatrix*vec4(vertexNormal, 0.0)).xyz;
	worldPos = (ModelMatrix*vec4(vertex.xyz, 1.0)).xyz;

	lightVec = lightPos - worldPos;
	eyeVec = viewPos - worldPos;

	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vertex;
}