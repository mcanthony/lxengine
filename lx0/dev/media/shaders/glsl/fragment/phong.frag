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

vec4 phong(vec3 ambient, vec3 diffuse, vec3 specular, float specularEx, float opacity);

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
