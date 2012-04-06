uniform mat4    unifViewMatrix;

void main(void)
{
    // Projection will occur in the geometry shader
    gl_Position = unifViewMatrix * gl_Vertex;
}
