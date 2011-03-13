#version 150

uniform sampler2D unifTexture0;

in vec3    fragVertexOc;
in vec3    fragColor;

out vec4 outColor;

const float PI = 3.1415926535;
const float BIAS = 0.001;

vec2 genUv(vec3 posOc)
{
    // Convert the object coordinate position into polar coordinates for the look-up
    // Add a small bias to limit edge sampling
    float theta = atan(posOc.y, posOc.x);
    float phi = atan(posOc.z, length(posOc.xy));
    float r = (PI/2.0 - phi) / PI - BIAS;
    return vec2(.5, .5) + vec2(r * cos(theta), r * sin(theta));
}

void main()
{	
    vec2 uv = genUv(fragVertexOc);
	outColor = texture2D(unifTexture0, uv);
}
