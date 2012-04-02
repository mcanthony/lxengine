#version 150
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D unifTexture0;

in  vec2 fragUV;
layout(location = 0) out vec4 outColor;

vec4 convolve()
{
    float kernel[5] = float[](0.20416368871516752, 0.18017382291138087, 0.1238315368057753, 0.0662822452863612, 0.027630550638898826);
    float dx = 1.0 / textureSize(unifTexture0, 0).x;

    vec4 sum = texture(unifTexture0, fragUV) * kernel[0];
    for(int i = 1; i < 5; i++)
    {
        vec2 offset = vec2(float(i), 0.0) * dx;
        sum += texture(unifTexture0, fragUV + offset) * kernel[i];
        sum += texture(unifTexture0, fragUV - offset) * kernel[i];
    }
    return sum;
}

void main()
{	   
    vec4 texColor = convolve();
    outColor = vec4(texColor.rgb, 1.0);
}
