in          vec2    vertUV;
varying out vec2    fragUV;

void main(void)
{   
    gl_Position = gl_Vertex;    
    fragUV      = vertUV;
}
