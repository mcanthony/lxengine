uniform mat4    unifProjMatrix;
uniform mat4    unifViewMatrix;
uniform mat3    unifNormalMatrix;

in      vec3    vertNormal;
in      vec2	vertUV;

varying out vec3 	fragNormal;
varying out vec2 	fragUV;
varying out float	fragIntensity;

void main()
{
    // Keep it simple and use a fixed light direction
    const vec3 lightDir = vec3(.5,-.5, 1.0);	
	
	fragNormal = unifNormalMatrix * vertNormal;
	fragIntensity = max( dot(normalize(lightDir), normalize(vertNormal)), 0.0);
	fragIntensity = smoothstep(0.2, 1.0, fragIntensity); 
	
	fragUV = vertUV;
	
    gl_Position = unifProjMatrix * unifViewMatrix * gl_Vertex;
}
