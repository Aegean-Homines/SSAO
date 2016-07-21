#version 330

uniform sampler2D gPositionDepth;
uniform int gBufDebug;
uniform int KernelSize;
uniform float NoiseSize;
uniform float Radius;

uniform vec3 SampleArray[128];
uniform mat4 ProjectionMatrix;

in vec2 texCoord;

void main()
{
	vec3 position = texture(gPositionDepth, texCoord).xyz;

	float AO = 0.0;

	for(int i = 0; i < KernelSize; i++){
		vec3 samplePos = position + SampleArray[i];
		vec4 offset = vec4(samplePos, 1.0);
		offset = ProjectionMatrix * offset; //Project to the back plane
		offset.xy /= offset.w; //perspective division
		offset.xy = offset.xy * 0.5 + vec2(0.5); // [0,1] range

		float sampleDepth = texture(gPositionDepth, offset.xy).b;

		/*float rangeCheck = smoothstep(0.0, 1.0, Radius / abs(position.z - sampleDepth));
		AO += (sampleDepth >= samplePos.z ? 1.0 : 0.0) * rangeCheck;*/

		if(abs(position.z - sampleDepth) < Radius){
			AO += step(sampleDepth, samplePos.z);
		}
	}

	AO = 1.0 - AO/128.0;

	gl_FragColor = vec4(pow(AO, 2.0));
}