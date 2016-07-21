#version 330

#define M_PI 3.1415926535897932384626433832795

uniform vec3 Ambient;
uniform vec3 Light;

uniform vec3 diffuse; //kd
uniform vec3 specular; //ks
uniform float shininess; //alpha
uniform bool IsAOEnabled;
uniform bool IsBlurred;
uniform bool isTextured;

in vec3 lightVec;
in vec3 eyeVec;
in vec2 texCoord;
in vec3 normalVec;

uniform sampler2D groundTexture;
uniform sampler2D ssaoFBO;
uniform sampler2D ssaoFBOBlurred;
uniform sampler2D gPosition;

uniform float Width;
uniform float Height;

vec2 CalcScreenTexCoord()
{
    return vec2(gl_FragCoord.x / Width, gl_FragCoord.y / Height);
}

void main(){

	gl_FragColor = vec4(texture(ssaoFBO, CalcScreenTexCoord()).x);
	return;

	vec3 N = normalize(normalVec);
	vec3 L = normalize(lightVec);
	vec3 V = normalize(eyeVec);
	vec3 H = normalize(L+V);

	float LN = max(dot(L,N), 0.0);
	float HN = max(dot(H,N), 0.0);
	float LH = dot(L,H);

	vec3 Kd;

	if(isTextured && textureSize(groundTexture, 0).x > 1){
		Kd = texture(groundTexture, texCoord.st).xyz;
	}else{
		Kd = diffuse; 
	}
		
	vec3 F = specular + (1 - specular) * pow((1 - LH), 5);
	float D = ((shininess + 2) / (2 * M_PI)) * pow(HN, shininess);
	float G = 1 / pow(LH, 2);

	vec3 BRDF = (Kd / M_PI) + (F * G * D) /4;

	vec4 FinalAmbient = vec4(Ambient, 1.0f);
	if(IsAOEnabled){
		FinalAmbient *= texture(ssaoFBO, CalcScreenTexCoord()).r;
	}

	gl_FragColor.xyz = BRDF * (Light) * LN + FinalAmbient.xyz * diffuse;
	
}