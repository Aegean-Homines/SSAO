#version 330

uniform vec3 ambientLight;

uniform sampler2D gPositionMap;
uniform sampler2D gNormalMap;
uniform sampler2D gSpecularMap;
uniform sampler2D gDifSpecMap;
uniform sampler2D depthMap;
uniform int gBufDebug;
uniform int Width;
uniform int Height;

in vec2 texCoord;

void main(){
	vec2 texCoords = vec2(gl_FragCoord.x/1024, gl_FragCoord.y/1024);

	gl_FragColor.rgb = texture(depthMap, texCoords).rgb;
}