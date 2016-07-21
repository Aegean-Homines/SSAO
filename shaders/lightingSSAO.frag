#version 330

#define M_PI 3.1415926535897932384626433832795

uniform vec3 AmbientLight;
uniform vec3 LightColor;

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D gDifSpec;
uniform sampler2D gSpecular;
uniform sampler2D ssaoFBO;
uniform sampler2D ssaoFBOBlurred;

uniform vec3 LightPosition;
uniform float Linear;
uniform float Quadratic;

uniform mat4 ViewInverse;

uniform int Width;
uniform int Height;

uniform bool IsAOEnabled;
uniform bool IsBlurred;

in vec2 texCoords;

void main(){
    vec2 texCoord = texCoords;

	vec3 position = texture(gPositionDepth, texCoord).rgb;
	vec3 normal = texture(gNormal, texCoord).rgb;
	vec3 diffuse = texture(gDifSpec, texCoord).rgb;
	float shininess = texture(gDifSpec, texCoord).a;
	vec3 specular = texture(gSpecular, texCoord).rgb;
	float AO;
	if(IsBlurred)
		AO = texture(ssaoFBOBlurred, texCoord).r;
	else
		AO = texture(ssaoFBO, texCoord).r;
	
	vec3 Ambient = AmbientLight;
	if(IsAOEnabled){
		Ambient *= AO;
	}


	// PHONG
    /*vec3 lighting  = Ambient; 
    vec3 viewDir  = normalize(-position); // Viewpos is (0.0.0)
    // Diffuse
    vec3 lightDir = normalize(LightPosition - position);
    vec3 diffPhong = max(dot(normal, lightDir), 0.0) * diffuse * LightColor;
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
    vec3 specPhong = LightColor * spec;
    // Attenuation
    lighting += diffPhong + specPhong;

	gl_FragColor = vec4(lighting, 1.0);*/
	
	// BRDF
	vec3 eyeVec = (ViewInverse*vec4(0,0,0,1)).xyz - position;
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

	gl_FragColor.xyz = BRDF * (LightColor) * LN + Ambient * diffuse;
	
}