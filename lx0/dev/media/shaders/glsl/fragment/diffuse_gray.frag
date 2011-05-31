#version 150

out vec4 outColor;

in vec3         fragVertexOc;
in vec3         fragVertexEc;
in vec3         fragNormalOc;
in vec3         fragNormalEc;
in vec3         fragColor;

void main()
{
    // Since the eye is at <0,0,0>, the direction vector to the vertex and 
    // the vertex position in eye coordinates are equivalent.
    //
    vec3 N = normalize(fragNormalEc);
    vec3 V = normalize(fragVertexEc);
       
    vec3 c = vec3(.05, .05, .05);           // ambient term

    vec3 L = normalize(vec3(1, 1, 1));
    float d = max(dot(N,L), 0.0);           // diffuse term
    c += vec3(d, d, d);
               
    outColor = vec4(c.x, c.y, c.z, 1.0);    	
}
