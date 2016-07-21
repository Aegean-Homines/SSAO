#version 330

#define G_POS 0
#define G_NORM 1
#define G_DIFF_XYZ 2
#define G_DIFF_W 3
#define G_SPEC 4
#define NONE 5

#define M_PI 3.1415926535897932384626433832795

uniform vec3 AmbientLight;

uniform sampler2D gPositionMap;
uniform sampler2D gNormalMap;
uniform sampler2D gSpecularMap;
uniform sampler2D gDifSpecMap;
uniform int gBufDebug;

uniform int Width, Height;

uniform float LightRange;

uniform mat4 ProjectionMatrix, ViewMatrix, ViewInverse;
uniform vec3 LightPosition;
uniform vec3 LightColor;
uniform vec2 Attenuation;

in vec2 texCoord;

out vec4 color;

void main(){
	vec2 texCoords = vec2(gl_FragCoord.x/Width, gl_FragCoord.y/Height);

	// Getting the information of texel back from the texture
	vec3 position = texture(gPositionMap, texCoords).rgb;
	vec3 normal = texture(gNormalMap, texCoords).rgb;
	vec3 diffuse = texture(gDifSpecMap, texCoords).rgb;
	float shininess = texture(gDifSpecMap, texCoords).a;
	vec3 specular = texture(gSpecularMap, texCoords).rgb;

	// Values to be used in BRDF
	vec3 eyeVec = (ViewInverse*vec4(0,0,0,1)).xyz - position;

	switch(gBufDebug){
	case G_POS:
		gl_FragColor.xyz = position;
		break;
	case G_NORM:
		gl_FragColor.xyz = abs(normal);
		break;
	case G_DIFF_XYZ:
		gl_FragColor.xyz = diffuse;
		break;
	case G_DIFF_W:
		gl_FragColor.xyz = vec3(shininess);
		break;
	case G_SPEC:
		gl_FragColor.xyz = specular;
		break;
	case NONE:
	default:
		// BRDF calculation
		float distance = length(LightPosition - position);
		if(distance < LightRange)
		{
			vec3 L = normalize(LightPosition - position);
			vec3 V = normalize(eyeVec);
			vec3 H = normalize(L+V);
			vec3 N = normalize(normal);

			float LN = max(dot(L,N), 0.0);
			float HN = max(dot(H,N), 0.0);
			float LH = dot(L,H);

			vec3 Kd = diffuse;

			vec3 F = specular + (1 - specular) * pow((1 - LH), 5);
			float D = ((shininess + 2) / (2 * M_PI)) * pow(HN, shininess);
			float G = 1 / pow(LH, 2);

			vec3 BRDF = (Kd / M_PI) + (F * G * D) /4;
			//float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
			//float attenuation = 1.0 / (1.0 + Attenuation[0] * distance + Attenuation[1] * (distance * distance));
			//float attenuation = 1.0;
			gl_FragColor.rgb = BRDF * (LightColor) * LN * ((LightRange - distance)/LightRange);
			//gl_FragColor.rgb = vec3(1.0, 0.0, 0.0);
			gl_FragColor.a = 1.0;
		}else{
			gl_FragColor.rgb = vec3(0.0, 0.0, 0.0);
			/*gl_FragColor.rgb = vec3(1.0);
			gl_FragColor.a = 1.0;*/
		}
		break;
	
	}
}