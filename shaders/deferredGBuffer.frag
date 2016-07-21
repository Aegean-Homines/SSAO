#version 330

in vec4 Vertex;
in vec3 VertexNormal;
in vec2 VertexTexture;
in vec3 VertexTangent;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDifSpec;
layout (location = 3) out vec3 gSpecular;

//in
in vec2 texCoord;
in vec3 normalVec;
in vec3 worldPos;

uniform int isTextured;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

// ground texture comes from the file
uniform sampler2D groundTexture;

void main()
{
	gPosition = worldPos;
	gNormal = normalize(normalVec);
	gDifSpec.a = shininess;

	if (isTextured == 1)
		gDifSpec.rgb = diffuse * texture(groundTexture, texCoord.st).rgb;
	else
		gDifSpec.rgb = diffuse;

	

	gSpecular = specular;
	
}
