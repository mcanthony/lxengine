#version 150
#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2D unifTexture0;

in  vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{	   
	vec4 texColor = texture2D(unifTexture0, fragUV);
    //if (outColor.a < .01)
    //discard;
    outColor = vec4(1 - texColor.r, 1 - texColor.g, 1 - texColor.b, 1.0);
}
