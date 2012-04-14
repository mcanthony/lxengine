uniform mat4    unifProjMatrix;
uniform mat4    unifViewMatrix;
uniform mat3    unifNormalMatrix;

in      vec3    vertNormal;
in		vec3	vertTangent;

varying out vec2  fragUV;
varying out vec3  fragNormal;
varying out vec3  fragTangent;

void main()
{   
    // The fragIntensity is effectively just the intensity of the diffuse 
    // value from the Phong reflection model.
    //
 	fragNormal = unifNormalMatrix * vertNormal;
    fragTangent = unifNormalMatrix * vertTangent;
	
    gl_Position = unifProjMatrix * unifViewMatrix * gl_Vertex;
	fragUV = gl_Vertex.xy;
}
