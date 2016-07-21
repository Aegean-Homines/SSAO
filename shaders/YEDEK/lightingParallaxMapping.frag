/////////////////////////////////////////////////////////////////////////
// Pixel shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330
#define M_PI 3.1415926535897932384626433832795

//in
in vec3 normalVec;
in vec2 texCoord;
in vec3 worldPos;
in vec3 originalLightVec; //L
in vec3 originalEyeVec; //V

// for normal map
in mat3 TBN;
in vec3 tangentLightPos;
in vec3 tangentViewPos;
in vec3 tangentWorldPos;

//uniform
uniform int mode;               // 0..9, used for debugging
uniform bool isTextured;
uniform bool isNormalMapped;
uniform bool isRight;
uniform bool isParallaxMappingEnabled;
uniform bool isSteepParallaxMappingEnabled;
uniform bool isParallaxOcclusionMappingEnabled;
uniform bool cropTextureMap;
uniform bool enhanceViewScaling;
uniform float heightScale;
uniform int depthLayerAmount;

uniform vec3 diffuse; //kd
uniform vec3 specular; //ks
uniform float shininess; //alpha
uniform vec3 Light;
uniform vec3 Ambient;

// ground texture comes from the file
uniform sampler2D groundTexture;
uniform sampler2D groundNormal;
uniform sampler2D depthMap;

vec2 ApplyParallaxMapping(vec2 texCoord, vec3 eyeVec);

void main()
{	
	vec3 lightVec = originalLightVec;
	vec3 eyeVec = originalEyeVec;
	vec2 textureCoordinates = texCoord;
	vec3 N = normalize(normalVec);

	if(isTextured && isNormalMapped){
		lightVec = tangentLightPos - tangentWorldPos;
		eyeVec = tangentViewPos - tangentWorldPos;

		if(isParallaxMappingEnabled){
			textureCoordinates = ApplyParallaxMapping(texCoord, normalize(originalEyeVec));
			if(cropTextureMap && textureCoordinates.x > 1.0 || textureCoordinates.y > 1.0 || textureCoordinates.x < 0.0 || textureCoordinates.y < 0.0)
				discard;
		}

		vec3 normal = texture2D(groundNormal, textureCoordinates).xyz;
		N = normalize(normal * 2.0 - 1.0);
	}

	vec3 L = normalize(lightVec);
	vec3 V = normalize(eyeVec);
	vec3 H = normalize(L+V);

	float LN = max(dot(L,N), 0.0);
	float HN = max(dot(H,N), 0.0);
	float LH = dot(L,H);

	vec3 Kd;

	if(isTextured && textureSize(groundTexture, 0).x > 1){
		Kd = texture(groundTexture, textureCoordinates.st).xyz;
	}else{
		Kd = diffuse; 
	}
		
	vec3 F = specular + (1 - specular) * pow((1 - LH), 5);
	float D = ((shininess + 2) / (2 * M_PI)) * pow(HN, shininess);
	float G = 1 / pow(LH, 2);

	vec3 BRDF = (Kd / M_PI) + (F * G * D) /4;

	gl_FragColor.xyz = BRDF * (Light) * LN + Ambient * diffuse;
}

vec2 ApplyParallaxMapping(vec2 texCoord, vec3 eyeVec){
	if(isSteepParallaxMappingEnabled){
		float layerInterval = 1.0 / depthLayerAmount;
		float currentLayerDepth = -layerInterval; // one less value for the initial loop - currentLayerDepth = 0

		vec2 p = eyeVec.xy * heightScale;
		vec2 texCoordInterval = p / depthLayerAmount;

		// iteration part
		vec2 currentTexCoord = texCoord + texCoordInterval; // one higher value for the initial loop - currentTexCoord = texCoord
		float currentDepth = 0.0;
		do{
			currentTexCoord -= texCoordInterval; // shift the tex coord along the P vector

			currentDepth = texture2D(depthMap, currentTexCoord).r;

			currentLayerDepth += layerInterval; //Update this with the new layer
		}while(currentLayerDepth < currentDepth);

		// AT THIS POINT COLLISION HAPPENED
		if(isParallaxOcclusionMappingEnabled){ //If we're using parallax mapping, apply this fix

			vec2 texCoordBeforeCollision = currentTexCoord + texCoordInterval; // Take back the last addition to find the previous location
			
			// These are the differences in the each side of the triangle (will be used in weight calculation)
			float afterCollisionDepth = currentDepth - currentLayerDepth;
			float beforeCollisionDepth = texture2D(depthMap, texCoordBeforeCollision).r - (currentLayerDepth -  layerInterval);

			float weight = afterCollisionDepth / (afterCollisionDepth - beforeCollisionDepth);
			currentTexCoord = texCoordBeforeCollision * weight + currentTexCoord * (1.0 - weight);
		}
		
		return currentTexCoord;
	}else{
		float depth =  texture2D(depthMap, texCoord).r;
		vec2 p;
		if(enhanceViewScaling)
			p = (eyeVec.xy / eyeVec.z) * (depth * heightScale);
		else
			p = eyeVec.xy * (depth * heightScale); //Parallax Mapping with Offset Limiting

		return texCoord - p;
	}
	
	
	
}