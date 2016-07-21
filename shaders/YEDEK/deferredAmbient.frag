#version 330

uniform vec3 ambientLight;

uniform sampler2D gPositionMap;
uniform sampler2D gNormalMap;
uniform sampler2D gSpecularMap;
uniform sampler2D gDifSpecMap;
uniform int gBufDebug;

in vec2 texCoord;

#define G_POS 0
#define G_NORM 1
#define G_DIFF_XYZ 2
#define G_DIFF_W 3
#define G_SPEC 4
#define NONE 5

out vec4 color;

void main(){

	vec3 outputColor;
	switch(gBufDebug){
	case G_POS:
		outputColor = texture(gPositionMap, texCoord.st).rgb;
		break;
	case G_NORM:
		outputColor = texture(gNormalMap, texCoord.st).rgb;
		break;
	case G_DIFF_XYZ:
		outputColor = texture(gDifSpecMap, texCoord.st).rgb;
		break;
	case G_DIFF_W:
		outputColor = texture(gDifSpecMap, texCoord.st).www;
		break;
	case G_SPEC:
		outputColor = texture(gSpecularMap, texCoord.st).rgb;
		break;
	case NONE:
	default:
		outputColor = texture(gDifSpecMap, texCoord.st).rgb;
		break;
	}
	color = vec4(outputColor * ambientLight, 1.0f);

	/*vec3 outputColor = texture(gDifSpecMap, texCoord.st).rgb; 
	color = vec4(outputColor * ambientLight, 1.0);*/
}