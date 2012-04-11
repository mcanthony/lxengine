uniform mat4    unifProjMatrix;
uniform mat4    unifViewMatrix;
uniform mat3    unifNormalMatrix;

in      vec3    vertNormal;

varying out vec3 	geomPositionWc;
varying out vec3 	geomNormal;
varying out float 	geomIntensity;

void main()
{
    // Keep it simple and use a fixed light direction
    const vec3 lightDir = vec3(.5,-.5, 1.0);
    
	geomNormal = unifNormalMatrix * vertNormal;
	geomIntensity = max( dot(normalize(lightDir), normalize(geomNormal)), 0.0);
    
	vec4 posWc = unifViewMatrix * gl_Vertex;
	geomPositionWc = posWc.xyz;
    gl_Position = unifProjMatrix * posWc;
}
