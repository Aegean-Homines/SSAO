/////////////////////////////////////////////////////////////////////////
// Pixel shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

in vec2 texCoord;

uniform vec3 diffuse; //kd
uniform float front; //near value of the projection matrix
uniform float back; //far value of the projection matrix
uniform sampler2D groundTexture;

float LinearDepthValue(float originalDepthValue);

void main()
{
	float final = LinearDepthValue(gl_FragCoord.z) / back;
	gl_FragColor = vec4(vec3(final), 1.0);
}

float LinearDepthValue(float originalDepthValue){
	float zVal = originalDepthValue * 2.0 - 1.0; //The same thing we did with the normal maps' normal value
	float linearizeDepth = (2.0 * front * back) / (back + front - zVal * (back - front)); //actual linearization
	return linearizeDepth;
}