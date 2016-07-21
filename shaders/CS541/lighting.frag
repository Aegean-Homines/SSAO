/////////////////////////////////////////////////////////////////////////
// Pixel shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330
#define M_PI 3.1415926535897932384626433832795

in vec4 Vertex;
in vec3 VertexNormal;
in vec2 VertexTexture;
in vec3 VertexTangent;

//in
in vec3 lightVec;
in vec3 eyeVec;
in vec2 texCoord;
in vec3 normalVec;

//uniform
uniform int mode;               // 0..9, used for debugging

uniform vec3 diffuse; //kd
uniform vec3 specular; //ks
uniform float shininess; //alpha
uniform vec3 Light;
uniform vec3 Ambient;

// ground texture comes from the file
uniform sampler2D groundTexture;

void main()
{	
	vec3 L = normalize(lightVec);
	vec3 V = normalize(eyeVec);
	vec3 H = normalize(L+V);
	vec3 N = normalize(normalVec);

	float LN = max(dot(L,N), 0.0);
	float HN = max(dot(H,N), 0.0);
	float LH = dot(L,H);

	vec3 Kd; //Diffuse

	if(textureSize(groundTexture, 0).x > 1){
		Kd = texture(groundTexture, texCoord.st).xyz;
	}else{
		Kd = diffuse; 
	}

	vec3 F = specular + (1 - specular) * pow((1 - LH), 5);
	float D = ((shininess + 2) / (2 * M_PI)) * pow(HN, shininess);
	float G = 1 / pow(LH, 2);
	
	vec3 BRDF = (Kd / M_PI) + (F * G * D) /4;

	gl_FragColor.xyz = BRDF * (Light) * LN + Ambient * diffuse;

}
