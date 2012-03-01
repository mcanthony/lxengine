uniform mat4    unifProjMatrix;
uniform mat4    unifViewMatrix;

void main(void)
{
    vec4 vertexEc = unifViewMatrix * gl_Vertex;
    gl_Position = unifProjMatrix * vertexEc;
}
