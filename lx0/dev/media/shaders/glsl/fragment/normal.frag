#version 150

out vec4 outColor;

uniform mat4    unifViewMatrix;

in vec3         fragVertexOc;
in vec3         fragVertexEc;
in vec3         fragNormalOc;
in vec3         fragNormalEc;
in vec3         fragColor;

void main()
{
    vec4 normalWc = vec4(fragNormalEc, 1.0) * unifViewMatrix;
    vec3 N = abs(normalize(normalWc.xyz));
    outColor = vec4(N.x, N.y, N.z, 1.0);    	
}
