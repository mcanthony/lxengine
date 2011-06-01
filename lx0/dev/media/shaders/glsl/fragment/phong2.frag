#version 150

out vec4 outColor;

uniform vec3    unifAmbient;

uniform int     unifLightCount;
uniform vec3    unifLightPosition[4];
uniform vec3    unifLightColor[4];

uniform vec3    unifMaterialDiffuse;

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
    vec3 V = normalize(fragVertexEc);
    vec3 R = reflect(V, N);
       
    vec3 c = ambient;                        // ambient term
    
    for (int i = 0; i < unifLightCount; ++i)
    {
        vec3 lightPosEc = unifLightPosition[i].xyz;

        vec3 L = lightPosEc - fragVertexEc;
        float d = length(L);
        L = normalize(L);
    
        float atten = 1;    // / (unifLightAtten[i].x + unifLightAtten[i].y * d  + unifLightAtten[i].x * d * d);
        vec3 lc = unifLightColor[i].rgb * atten;
    
        c += diffuse * lc * unifMaterialDiffuse * max(dot(N,L), 0.0);                   // diffuse term
        c += lc * specular * pow(max(dot(R,L),0.0), specularEx);  // specular term
    }
               
    return c;
}

void main()
{              
    outColor = vec4( phong(unifAmbient, fragColor, vec3(1.0, 1.0, 1.0), 32), 1.0);    	
}
