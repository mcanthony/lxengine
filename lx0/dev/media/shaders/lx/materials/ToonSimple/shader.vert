uniform mat4    unifProjMatrix;
uniform mat4    unifViewMatrix;
uniform mat3    unifNormalMatrix;

in      vec3    vertNormal;

varying out float fragIntensity;

void main()
{
    vec3 lightDir = vec3(.5,-.5, 1.0);
	
    fragIntensity = dot(normalize(lightDir), unifNormalMatrix * vertNormal);
	gl_Position = unifProjMatrix * unifViewMatrix * gl_Vertex;
}
