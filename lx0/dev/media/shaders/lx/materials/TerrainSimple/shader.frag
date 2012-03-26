#version 150
#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2D unifTexture0;
uniform sampler2D unifTexture1;
uniform sampler2D unifTexture2;
uniform sampler2D unifTexture3;

in  float fragIntensity;
in  vec3  fragUVW;

layout(location = 0) out vec4 outColor;

void main()
{
	float i = (.5 * fragIntensity + .5);
	
    vec4 c0 = texture(unifTexture0, 16 * fragUVW.xy);
	vec4 c1 = texture(unifTexture1, 12 * fragUVW.xy);
    vec4 c2 = texture(unifTexture2, 14 * fragUVW.xy);
    float n = texture(unifTexture2, .5 * fragUVW.xy).r;

    if (fragUVW.z > .2)
        c2 = mix(c2, vec4(1,1,1,1), n * (fragUVW.z - .2) * 8);
       
    outColor =  i * mix(mix(c0, c1, n), c2, fragUVW.z * 4);
}
