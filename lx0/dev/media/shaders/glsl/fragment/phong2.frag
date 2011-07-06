#version 150

out vec4 outColor;

uniform vec3    unifAmbient;

uniform int     unifLightCount;
uniform vec3    unifLightPosition[4];
uniform vec3    unifLightColor[4];

uniform vec3    unifMaterialDiffuse;
uniform vec3    unifMaterialSpecular;
uniform float   unifMaterialSpecularEx;

uniform mat4    unifViewMatrix;

in vec3         fragVertexOc;
in vec3         fragVertexEc;
in vec3         fragNormalOc;
in vec3         fragNormalEc;
in vec3         fragColor;

vec3 phong(vec3 ambient, vec3 diffuse, vec3 specular, float specularEx)
{
    // Since the eye is at <0,0,0>, the direction vector to the vertex and 
    // the vertex position in eye coordinates are equivalent.
    //
    vec3 N = normalize(fragNormalEc);
    vec3 V = -normalize(fragVertexEc);
       
    vec3 c = ambient;                        // ambient term
    
    for (int i = 0; i < unifLightCount; ++i)
    {
        vec3 L = unifLightPosition[i].xyz - fragVertexEc;
        float d = length(L);
        L = normalize(L);
    
        float atten = 1;    // / (unifLightAtten[i].x + unifLightAtten[i].y * d  + unifLightAtten[i].x * d * d);
        vec3 lc = unifLightColor[i].rgb * atten;
    
        c += lc * diffuse * max(dot(N,L), 0.0);                   // diffuse term
        
        vec3 H = normalize(L + V);
        c += lc * specular * pow(max(dot(N,H),0.0), specularEx);  // specular term
    }
               
    return c;
}

void main()
{              
    outColor = vec4( phong(unifAmbient, unifMaterialDiffuse, unifMaterialSpecular, unifMaterialSpecularEx), 1.0);   
}
