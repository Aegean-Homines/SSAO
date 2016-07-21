/////////////////////////////////////////////////////////////////////////
// Pixel shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

layout (location = 0) out vec4 gPositionDepth;
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
uniform float front; //near value of the projection matrix
uniform float back; //far value of the projection matrix
uniform sampler2D groundTexture;

float LinearDepthValue(float originalDepthValue);

void main()
{	
	gPositionDepth.xyz = worldPos;
	gPositionDepth.a = LinearDepthValue(gl_FragCoord.z);
	gNormal = normalize(normalVec);
	gDifSpec.a = shininess;

	if (isTextured == 1)
		gDifSpec.rgb = diffuse * texture(groundTexture, texCoord.st).rgb;
	else
		gDifSpec.rgb = diffuse;

	gSpecular = specular;
}

float LinearDepthValue(float originalDepthValue){
	float zVal = originalDepthValue * 2.0 - 1.0; //The same thing we did with the normal maps' normal value
	float linearizeDepth = (2.0 * front * back) / (back + front - zVal * (back - front)); //actual linearization
	return linearizeDepth;
}