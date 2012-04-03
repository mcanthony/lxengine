#version 150
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D unifTexture0;
uniform float     unifCurrentTime;

in  vec2 fragUV;
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265358979323846264;

//
// Modulate the UV coordinates according to an animated sin wave.
// 
vec2 wave (vec2 uv)
{
    float y = fract(20 * uv[1] + (unifCurrentTime / 2000.0));
    uv[0] += .005 * sin(2 * PI * y);	
    return uv;
}

void main()
{	   
	vec2 uv = wave(fragUV);
    vec4 texColor = texture2D(unifTexture0, uv);
    if (texColor.a < .01)
        discard;
    outColor = vec4(texColor.rgb, 1.0);
}
