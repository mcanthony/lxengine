#version 120

in vec3         fragVertexOc;
in vec3         fragVertexEc;
in vec3         fragNormalOc;
in vec3         fragNormalEc;
in vec3         fragColor;

uniform vec4 inColor;
uniform float unifLightCount;
uniform vec3 unifAmbient;
uniform vec3 unifDiffuse;
uniform vec3 unifSpecular;

vec4 phong(vec3 ambient, vec3 diffuse, vec3 specular, float specularEx, float opacity)
{
    // Since the eye is at <0,0,0>, the direction vector to the vertex and 
    // the vertex position in eye coordinates are equivalent.
    //
    vec3 N = normalize(fragNormalEc);
    vec3 V = normalize(fragVertexEc);
    vec3 R = reflect(V, N);
       
    vec3 c = ambient;                        // ambient term
    
    for (int i = 0; i < unifLightCount; ++i)
    {
        vec3 unifLightPosEc = gl_LightSource[i].position.xyz;

        vec3 L = unifLightPosEc - fragVertexEc;
        float d = length(L);
        L = normalize(L);
    
        float atten = 1;// / (unifLightAtten[i].x + unifLightAtten[i].y * d  + unifLightAtten[i].x * d * d);
        vec3 lc = gl_LightSource[i].diffuse.xyz * atten;
    
        c += diffuse * lc * max(dot(N,L), 0.0);                   // diffuse term
        c += lc * specular * pow(max(dot(R,L),0.0), specularEx);  // specular term
    }
               
    return vec4(c.x, c.y, c.z, opacity);    
}

void main()
{	
    gl_FragColor = phong(
        unifAmbient,
        unifDiffuse,
        unifSpecular,
        32,
        1.0
        );
}
