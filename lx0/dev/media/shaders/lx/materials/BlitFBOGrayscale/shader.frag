#version 150
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D unifTexture0;

in  vec2 fragUV;
layout(location = 0) out vec4 outColor;

// Function from OpenGL 4.0 Shading Language Cookbook
float luminance (vec3 color) 
{
    return dot( color.rgb, vec3(0.2126, 0.7152, 0.0722) );
}

void main()
{	   
    vec4 texColor = texture2D(unifTexture0, fragUV);
    if (texColor.a < .01)
        discard;

    float gray = luminance(texColor.rgb);
    outColor = vec4(mix(texColor.rgb, vec3(gray,gray,gray), .95), 1.0);
}
