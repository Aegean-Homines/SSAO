/////////////////////////////////////////////////////////////////////////
// Pixel shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330
#define M_PI 3.1415926535897932384626433832795

//in
in vec3 normalVec;
in vec3 lightVec;
in vec3 eyeVec;
in vec2 texCoord;
in vec3 worldPos;
in vec4 shadowCoord;

//uniform
uniform int mode;               // 0..9, used for debugging
uniform float groundRadius;
uniform float lightDistance;
uniform int shadowDebug;
uniform float C;

uniform vec3 diffuse; //kd
uniform vec3 specular; //ks
uniform float shininess; //alpha
uniform vec3 Light;
uniform vec3 Ambient;

// ground texture comes from the file
uniform sampler2D groundTexture;
// shadow texture coming from the shadow FBO
uniform sampler2D shadowMap;
uniform sampler2D blurredShadowMap;

#define	PIXEL_DEPTH 0
#define	PIXEL_DEPTH_MAPPED 1
#define LIGHT_DEPTH 2
#define LIGHT_DEPTH_MAPPED 3
#define LIGHT_DEPTH_FROM_TEXTURE 4
#define	SHADOW_COLOR 5
#define LIGHT_DEPTH_LOGARITHMIC 6
#define EXPONENTIAL_PIXEL_DEPTH 7
#define VISIBILITY 8
#define	NONE_SHADOW 9

float mapValue(float value, float min, float max){
	return (value - min) / (max - min);
}

float saturate(float value){
	return clamp(value, 0.0, 1.0);
}

void main()
{	
	vec3 N = normalize(normalVec);
	vec3 L = normalize(lightVec);
	vec3 V = normalize(eyeVec);
	vec3 H = normalize(L+V);

	float LN = max(dot(L,N), 0.0);
	float HN = max(dot(H,N), 0.0);
	float LH = dot(L,H);

	vec3 Kd; //Diffuse
	
	if(textureSize(groundTexture, 0).x > 1){
		Kd = texture(groundTexture, texCoord.st).xyz;
	}else{
		Kd = diffuse;
	}
	
	//*********** SHADOW PART ***************

	bool inShadow = false;
	vec2 shadowIndex = shadowCoord.xy / shadowCoord.w;
	// The value of this pixel in the shadowMap
	// Basically lightDepth = the pixel distance from Light's view (distance between Light and the occluder)
	float lightDepth = texture2D(blurredShadowMap, shadowIndex).r;
	// Distance between light and the current fragment
	float logLightDepth = log(lightDepth);

	float pixelDepth = shadowCoord.w;
	float mappedPixelDepth = mapValue(pixelDepth, lightDistance - groundRadius, lightDistance + groundRadius);

	float exponentialPixelDepth = exp(-C * mappedPixelDepth);
	float visibility = saturate(lightDepth * exponentialPixelDepth);

		
	float occluder = textureProj(blurredShadowMap, shadowCoord.xyw).r;
	float receiver = mapValue(pixelDepth, lightDistance - groundRadius, lightDistance + groundRadius);

	visibility = saturate(occluder * exp(-C * receiver));

	// bias to be used to prevent floating point calculation errors
	// doing so stops shadow acne problem
	float bias = 0.000001;
	if(shadowCoord.w > 0 && shadowIndex.x < 1 && shadowIndex.x > 0 && shadowIndex.y < 1 && shadowIndex.y > 0){
		if(mappedPixelDepth > lightDepth + bias){
			inShadow = true;
		}else{
			inShadow = false;
		}
	}else{ // This is for defining areas out of shadow frustum. For this assignment, the light source is like a directional light 
		inShadow = true;
	}

	//*********** BRDF PART *****************

	switch(shadowDebug){
	case PIXEL_DEPTH: //pixel Depth Debugging
		gl_FragColor.xyz = vec3(pixelDepth / 100.0);
		break;
	case PIXEL_DEPTH_MAPPED:
		gl_FragColor.xyz = vec3(mappedPixelDepth);
		break;
	case LIGHT_DEPTH:
		gl_FragColor.xyz = vec3(lightDepth / 100.0);
		break;
	case LIGHT_DEPTH_MAPPED:
		gl_FragColor.xyz = vec3(0.0);
		break;
	case LIGHT_DEPTH_FROM_TEXTURE:
		gl_FragColor.xyz = texture2D(blurredShadowMap, shadowIndex).xyz;
		break;
	case SHADOW_COLOR: // shadow color debugging
		gl_FragColor.xy = shadowIndex;
		gl_FragColor.z = 0;
		break;
	case LIGHT_DEPTH_LOGARITHMIC:
		gl_FragColor.xyz = vec3(logLightDepth);
		break;
	case EXPONENTIAL_PIXEL_DEPTH:
		gl_FragColor.xyz = vec3(exponentialPixelDepth);
		break;
	case VISIBILITY:
		gl_FragColor.xyz = vec3(visibility); 
		break;
	case NONE_SHADOW: // clear debugging
		if(inShadow){ // if the fragment is in the shadow area, I don't need to calculate BRDF - just multipyling color with ambient light
			gl_FragColor.xyz = Ambient * diffuse; // Default value
			//gl_FragColor.xyz = vec3(1.0f, 0.8f, 0.2f); //For debugging
		}else{
			vec3 F = specular + (1 - specular) * pow((1 - LH), 5);
			float D = ((shininess + 2) / (2 * M_PI)) * pow(HN, shininess);
			float G = 1 / pow(LH, 2);
	
			vec3 BRDF = (Kd / M_PI) + (F * G * D) /4;

			BRDF *= visibility; 

			BRDF = max(BRDF, vec3(0.0));

			gl_FragColor.xyz = BRDF * (Light) * LN + Ambient * diffuse;
		}
	}

}
