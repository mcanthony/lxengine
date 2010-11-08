#version 120

in vec3         fragVertexOc;
in vec3         fragVertexEc;
in vec3         fragNormalOc;
in vec3         fragNormalEc;
in vec3         fragColor;

uniform vec3 unifAmbient;
uniform vec3 unifDiffuse;
uniform vec3 unifSpecular;

vec2 uvCyclindrical (vec3 vertexOc, vec2 scale);
vec3 checker(vec2 uv, vec3 primary, vec3 secondary);
vec4 phong(vec3 ambient, vec3 diffuse, vec3 specular, float specularEx, float opacity);

void main()
{	
    gl_FragColor = phong(
        unifAmbient,
        checker(uvCyclindrical(fragVertexOc, vec2(2, 2)), unifDiffuse, unifSpecular),
        unifSpecular,
        32,
        1.0
        );
}
