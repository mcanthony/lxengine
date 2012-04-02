
uniform     mat4    unifProjMatrix;
uniform     mat4    unifViewMatrix;

in          vec2    vertUV;

varying out vec2    fragUV;

void main(void)
{   
    gl_Position = unifProjMatrix * unifViewMatrix * gl_Vertex;    
    fragUV      = vertUV;
}
