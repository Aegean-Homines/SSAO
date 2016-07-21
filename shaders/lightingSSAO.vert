#version 330

layout (location = 0) in vec4 vertPosition;
layout (location = 1) in vec3 vertColor;
layout (location = 2) in vec3 vertNormal;
layout (location = 3) in vec3 vertTexCoord;

uniform mat4 ProjectionMatrix, ViewMatrix, ModelMatrix;

out vec2 texCoords;

void main(){
	texCoords = vec2(vertTexCoord.x, vertTexCoord.y);
	gl_Position = vertPosition;

	/*texCoords = vec2(vertTexCoord.x, vertTexCoord.y);
	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vertPosition;*/
}