#version 150

in vec3 fragColor;
in vec3 fragNormalEc;
in vec3 fragLightDirEc;

out vec4 outColor;

void main()
{	    
    vec3 N = normalize(fragNormalEc);
    float d = max(N.z, 0.0) * .75 + .25;   // diffuse term

	outColor = vec4(d * fragColor.xyz, 1.0);
}
