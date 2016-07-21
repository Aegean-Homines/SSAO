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
in vec3 normalVec;
in vec3 lightVec;
in vec3 eyeVec;
in vec2 texCoord;
in vec4 shadowCoord;

//uniform
uniform sampler2D groundTexture;
uniform vec3 diffuse; //kd
uniform vec3 specular; //ks
uniform float shininess; //alpha
uniform vec3 Light;
uniform vec3 Ambient;

uniform sampler2D NormalMap;
uniform sampler2D BumpMap;
uniform sampler2D shadowMap;
uniform sampler2D skyDome;

void main()
{	

	vec3 N;

	if(textureSize(NormalMap, 0).x > 1){
		vec3 normalMap = (texture2D(NormalMap, texCoord)).rgb * 2.0 - 1.0;
		vec3 B = cross(VertexTangent, VertexNormal);
		N = normalMap.x * VertexTangent + normalMap.y * B + normalMap.z * VertexNormal;
	}else{
		N = normalize(normalVec);
	}

	vec3 L = normalize(lightVec);
	vec3 V = normalize(eyeVec);
	vec3 H = normalize(L+V);
	
	// Used at multiple places so we're calculating them once
	float LN = max(dot(L,N), 0.0);
	float HN = max(dot(H,N), 0.0);
	float LH = dot(L,H);

	if (textureSize(skyDome, 0).x>1) {
		vec3 D = normalize(2*dot(N, L)*N - L);
		vec2 uv = vec2(1/2-atan(D.y,D.x)/(2*M_PI), acos(D.z)/M_PI);
		gl_FragColor.xyz = texture(skyDome, uv.st).xyz;
		return; //again, we're just applying texture, no need to go further
	}

	vec3 Kd;
	if(textureSize(groundTexture, 0).x > 1){
		Kd = texture(groundTexture, texCoord.st).xyz;
	}else{
		Kd = diffuse;
	}

	// Don't really need these for this assignment but I included it to also reflect the shadows
	bool inShadow = false;
	vec2 shadowIndex = shadowCoord.xy / shadowCoord.w;
	float lightDepth = texture2D(shadowMap, shadowIndex).w;
	float pixelDepth = shadowCoord.w / 100.0f;

	if(shadowCoord.w > 0 && shadowIndex.x < 1 && shadowIndex.x > 0 && shadowIndex.y < 1 && shadowIndex.y > 0){
		if(pixelDepth > lightDepth + 0000.1){
			inShadow = true;
		}else{
			inShadow = false;
		}
	}else{
		inShadow = false;
	}

	if(inShadow){
		gl_FragColor.xyz = Ambient * diffuse;
	}else{
		vec3 F = specular + (1 - specular) * pow((1 - LH), 5);
		float D = ((shininess + 2) / (2 * M_PI)) * pow(HN, shininess);
		float G = 1 / pow(LH, 2);
	
		vec3 BRDF = (Kd / M_PI) + (F * G * D) /4;

		gl_FragColor.xyz = BRDF * (Light) * LN + Ambient * diffuse;
	}

}
