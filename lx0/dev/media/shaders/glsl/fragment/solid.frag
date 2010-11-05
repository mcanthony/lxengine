#version 150

uniform vec4 inColor;

out vec4 outColor;

void main()
{	
	outColor = vec4(inColor.xyz, 1.0);
}
