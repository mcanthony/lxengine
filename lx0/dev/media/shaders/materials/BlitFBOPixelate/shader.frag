#version 150
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D unifTexture0;

in  vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{	   
	vec2 size = textureSize(unifTexture0, 0) / 6;
	vec2 uv = floor(fragUV * size + .5) / size;
    vec4 texColor = texture2D(unifTexture0, uv);
    
	vec3 levels = vec3(8, 16, 6);
	texColor.rgb = floor(texColor.rgb * levels) / levels;
	texColor = mix(texColor, texture2D(unifTexture0, fragUV), .25);
	
    if (texColor.a < .01)
        discard;
    outColor = vec4(texColor.r, texColor.g, texColor.b, 1.0);
}
