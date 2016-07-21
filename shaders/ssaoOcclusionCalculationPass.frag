#version 330

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D ssaoNoise;
uniform int gBufDebug;
uniform float Width;
uniform float Height;
uniform int KernelSize;
uniform float NoiseSize;
uniform float Radius;

uniform vec3 SampleArray[64];
uniform mat4 ProjectionMatrix;

in vec2 texCoord;

out float FragColor;

void main()
{
	vec2 texOffsetScale = vec2(Width / NoiseSize, Height / NoiseSize);
	vec3 fragmentPosition = texture(gPositionDepth, texCoord).xyz;
	vec3 randomizeVec = texture(ssaoNoise, texCoord * texOffsetScale).xyz;

	// Creating TBN matrix = we need to take everything from tangent-space to view-space
	vec3 N = texture(gNormal, texCoord).xyz;
	vec3 T = normalize(randomizeVec - N * dot(randomizeVec, N));
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N);

	float calculatedOcclusionValue = 0.0f;
	vec3 sample;
	vec4 sampleOffset;
	float sampledDepth;
	float rangeCheckResult;
	for(int i = 0; i < KernelSize; ++i){
		sample = TBN * SampleArray[i]; // We take the value from the array we sent from cpu and transform it to view space
		sample = fragmentPosition + sample * Radius;

		// Sample is still in view-space though - we need to transform it to screen-space so we apply projection
		sampleOffset = vec4(sample, 1.0);
		sampleOffset = ProjectionMatrix * sampleOffset;
		sampleOffset.xyz /= sampleOffset.w; //Homogenous division
		sampleOffset.xyz = sampleOffset.xyz * 0.5 + vec3(0.5); // range transformation to [0,1] to use in sampling

		// sampledDepth = Sampled from the linear depth texture. It is the depth of sampled position FROM viewer's perspective => NON-OCCLUDED
		sampledDepth = -texture(gPositionDepth, sampleOffset.xy).w;

		// ISSUE: When we're sampling edges, it'll also take background to the account. To remove it we're doing this calculation
		// smoothstep => interpolate third param between first and second
		rangeCheckResult = smoothstep(0.0, 1.0, Radius / abs(fragmentPosition.z - sampledDepth));

		//calculatedOcclusionValue += (sampledDepth >= sample.z ? 1.0 : 0.0);
		calculatedOcclusionValue += (sampledDepth >= sample.z ? 1.0 : 0.0) * rangeCheckResult;
	}

	calculatedOcclusionValue = 1.0 - (calculatedOcclusionValue / KernelSize); // Division => normalizing occlusion value. Subtraction => using the occlusion factor directly on ambient light component
	FragColor = calculatedOcclusionValue;
}