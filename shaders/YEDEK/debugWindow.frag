#version 330

in  vec2 texCoord;
  
uniform sampler2D fboToDebug;
  
void main()
{
	//float depth = texture2D(fboToDebug, texCoord).x;
    //gl_FragColor = vec4(log(depth)/100.0); //USE THIS ONE FOR SOFT SHADOW
	//gl_FragColor = vec4(depth); //USE THIS ONE FOR NORMAL SHADOW
	gl_FragColor = texture(fboToDebug, texCoord); // USE THIS ONE FOR THE REST OF IT
} 