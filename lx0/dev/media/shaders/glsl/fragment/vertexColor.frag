#version 150

in  vec3 fragColor;
out vec4 outColor;

void main()
{	
	outColor = vec4(fragColor.xyz, 1.0);
}
