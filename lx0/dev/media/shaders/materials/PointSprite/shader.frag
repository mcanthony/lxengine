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
    outColor = texture(unifTexture0, fragUV);
}
