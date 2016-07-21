#version 330

in vec2 texCoord;

out float FragColor;

uniform sampler2D ssaoTexture;
uniform int noiseTextureSize;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(ssaoTexture, 0));
	float result = 0.0;
	vec2 textureOffset;
	for(int x = 0; x < noiseTextureSize; ++x){
		for(int y = 0; y < noiseTextureSize; ++y){
			textureOffset = (vec2(-2,0) + vec2(float(x), float(y))) * texelSize;
			result += texture(ssaoTexture, texCoord + textureOffset).r;
		}
	}

	FragColor = result / float(noiseTextureSize * noiseTextureSize);
}