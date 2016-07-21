/////////////////////////////////////////////////////////////////////////
// Pixel shader for the final pass
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

in vec4 position; 

void main()
{
	gl_FragData[0] = vec4(position.w / 100.0f, position.w / 100.0f, position.w / 100.0f, position.w / 100.0f);
}
