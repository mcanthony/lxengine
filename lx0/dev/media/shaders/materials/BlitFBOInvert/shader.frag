#version 150
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D unifTexture0;

in  vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{	   
    vec4 texColor = texture2D(unifTexture0, fragUV);
    if (texColor.a < .01)
        discard;
    outColor = vec4(vec3(1,1,1) - texColor.rgb, 1.0);
}
