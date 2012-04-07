#version 150

uniform vec3 unifColor;

out vec4 outColor;

void main()
{	
	outColor = vec4(unifColor.rgb, 1.0);
}
