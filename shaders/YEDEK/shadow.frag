/////////////////////////////////////////////////////////////////////////
// Pixel shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

in vec4 position; 

uniform float C;
uniform float groundRadius;
uniform float lightDistance;

float mapValue(float value, float min, float max){
	return (value - min) / (max - min);
}

void main()
{
	/*float depth = position.w;
	gl_FragColor = vec4(position.w / 100.0);*/
	/*depth = mapValue(depth, lightDistance - groundRadius, lightDistance + groundRadius);
	float exponentialValue = exp(C * depth);
	gl_FragData[0].r = exponentialValue;*/

	float depth = (1.0 / gl_FragCoord.w);
	float mappedDepth = mapValue(depth, lightDistance - groundRadius, lightDistance + groundRadius);

	gl_FragData[0].r = exp(C * mappedDepth);

}
