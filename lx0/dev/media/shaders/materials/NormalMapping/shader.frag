#version 150
#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2D unifDiffuseMap;
uniform sampler2D unifNormalMap;

in  vec2  fragUV;
in  vec3  fragNormal;
in  vec3  fragTangent;

layout(location = 0) out vec4 outColor;

void main()
{
    // Keep it simple and use a fixed light direction
    vec3 lightDir = vec3(.5,-.5, 1.0);
	
	vec3 t = normalize( fragTangent );
	vec3 n = normalize( fragNormal );
	vec3 b = normalize( cross(n,t) );
	
    vec3 normalMap = normalize( texture(unifNormalMap, 4 * fragUV).xyz * 2 - 1.0);
    
	vec3 L = -lightDir;
	vec3 light = vec3(dot(t,L), dot(b,L), dot(n,L));
    float i0 = dot(normalize(light), normalMap) * .5 + .5;	
       
	outColor = vec4(i0 * texture(unifDiffuseMap, 4 * fragUV).xyz, 1);
}
