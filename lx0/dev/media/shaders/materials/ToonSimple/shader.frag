#version 150
#extension GL_ARB_explicit_attrib_location : enable

uniform sampler1D unifTexture0;

in  float fragIntensity;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(unifTexture0, fragIntensity);        
}
