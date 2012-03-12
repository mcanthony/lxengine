#version 150
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D unifTexture0;

in  vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{	   
    vec2 d = pow(vec2(1,1) + 2 * abs(fragUV - vec2(.5, .5)), vec2(4,4));
    vec2 ex = d / textureSize2D(unifTexture0, 0);

	vec4 texColor = texture2D(unifTexture0, fragUV);
    texColor += texture2D(unifTexture0, fragUV + ex);
    texColor += texture2D(unifTexture0, fragUV - ex);
    texColor += texture2D(unifTexture0, fragUV + vec2(-ex.x, ex.y));
    texColor += texture2D(unifTexture0, fragUV + vec2(ex.x, -ex.y));
    
    texColor /= 5.0;
    
    if (texColor.a < .01)
        discard;
    outColor = vec4(1 - texColor.r, 1 - texColor.g, 1 - texColor.b, 1.0);
}
