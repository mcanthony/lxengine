
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
    vec4 vertexEc = gl_ModelViewMatrix * gl_Vertex;
    geomVertexEc = vertexEc.xyz;
    
    // The Projection matrix
    gl_Position = gl_ProjectionMatrix * vertexEc;

    geomNormalOc = gl_Normal;
    geomNormalEc = gl_NormalMatrix * gl_Normal;
    geomColor  = vec3(gl_Color.xyz);        
}
