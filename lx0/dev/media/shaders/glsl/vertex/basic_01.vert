
uniform mat4    unifProjMatrix;
uniform mat4    unifViewMatrix;
uniform mat3    unifNormalMatrix;

in vec3 vertNormal;
in vec3 vertColor;

varying out vec3    geomLightDirEc;

varying out vec3    geomVertexOc;
varying out vec3    geomVertexEc;
varying out vec3    geomNormalOc;
varying out vec3    geomNormalEc;
varying out vec3    geomColor;

void main(void)
{
    //
    // The ModelView matrix converts coordiantes from object coordinates to
    // eye coordinates.  The intermediate world coordinate system is implicit
    // in the combined Model+View matrix.
    //
    // The eye coordinate system places the "eye" at 0,0,0 looking down the
    // -Z axis.  The visible portion of this coordinate system depends on
    // the value of the gl_ProjectionMatrix, which makes eye space to clip
    // coordinates.
    //
    // (Clip coordinates become normalized device coordinates after the 
    // perspective divide; NDCs are then mapped to display coordinates by
    // the glViewport settings.) 
    //
    geomVertexOc = gl_Vertex;
    vec4 vertexEc = unifViewMatrix * gl_Vertex;
    geomVertexEc = vertexEc.xyz / vertexEc.w;
    
    // The Projection matrix
    gl_Position = unifProjMatrix * vertexEc;

    geomNormalOc = vertNormal;
    geomNormalEc = unifNormalMatrix * vertNormal;
    geomColor  = vertColor;
    
    geomLightDirEc = normalize(vec3(-1, 1, -1));
}
