#version 150
#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2D unifTexture0;
uniform sampler2D unifTexture1;

in  float fragIntensity;
in  vec3  fragUVW;

layout(location = 0) out vec4 outColor;

void main()
{
	float i = (.5 * fragIntensity + .5);
	vec4 c0 = texture(unifTexture0, 16 * fragUVW.xy);
	vec4 c1 = texture(unifTexture1, 12 * fragUVW.xy);
    outColor =  i * mix(c0, c1, fragUVW.z * 4);
}
