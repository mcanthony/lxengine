uniform mat4    unifProjMatrix;
uniform mat4    unifViewMatrix;
uniform mat3    unifNormalMatrix;

uniform vec3      unifBBoxMin;
uniform vec3      unifBBoxMax;

in      vec3    vertNormal;

varying out float fragIntensity;
varying out vec2  fragUV;
varying out float fragHeight;

void main()
{
    // Keep it simple and use a fixed light direction
    vec3 lightDir = vec3(.5,-.5, 1.0);
    
    // The fragIntensity is effectively just the intensity of the diffuse 
    // value from the Phong reflection model.
    //
    fragIntensity = dot(normalize(lightDir), unifNormalMatrix * vertNormal);
    
    gl_Position = unifProjMatrix * unifViewMatrix * gl_Vertex;
	fragUV = gl_Vertex.xy;
    fragHeight = (gl_Vertex.z - unifBBoxMin.z) / (unifBBoxMax.z - unifBBoxMin.z);
}
