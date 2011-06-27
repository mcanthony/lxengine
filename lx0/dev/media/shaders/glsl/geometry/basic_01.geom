#version 150 
#extension GL_EXT_geometry_shader4 : enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform int         unifFlatNormals;
uniform sampler1D   unifFaceFlags;
uniform int         unifFaceCount;

in vec3             geomLightDirEc[];

in vec3             geomVertexOc[];
in vec3             geomVertexEc[];
in vec3             geomNormalOc[];
in vec3             geomNormalEc[];
in vec3             geomColor[];

out vec3            fragLightDirEc;

out vec3            fragVertexOc;
out vec3            fragVertexEc;
out vec3            fragNormalOc;
out vec3            fragNormalEc;
out vec3            fragColor;

vec3 computeNormal (vec3 v0, vec3 v1, vec3 v2)
{
    vec3 a = v2 - v1;
    vec3 b = v1 - v0;
    return normalize( cross (b, a) );
}

void main()
{	
    // Compute a normal based on the adjacent vertices in the triangle.
    // By definition, this will produce a uniform normal for the entire
    // triangle rather than smooth shading.
    //
    // gl_PositionIn[] is in clip coordinates.  The normal should be
    // computed in eye coordinates.
    //
    vec3 n = computeNormal(geomVertexEc[0], geomVertexEc[1], geomVertexEc[2]);
    
    for (int i = 0; i < gl_VerticesIn; i++)
    {
        fragLightDirEc = geomLightDirEc[i];
        
        gl_Position = gl_PositionIn[i];
        
        fragVertexOc = geomVertexOc[i];
        fragVertexEc = geomVertexEc[i];
        
        int flatFace = (unifFlatNormals == 0) ? 0 : 1;
        if (unifFaceCount > 0)
        {
            float s = (gl_PrimitiveIDIn + .5) / textureSize(unifFaceFlags, 0);
            float f = textureLod(unifFaceFlags, s, 0).x;
            flatFace = (f > 0.5) ? 0 : 1;
        }

        if (flatFace != 0)
        {
            fragNormalEc = n;
            fragNormalOc = computeNormal(geomVertexOc[0], geomVertexOc[1], geomVertexOc[2]);
        }
        else
        {
            fragNormalEc = normalize(geomNormalEc[i]);
            fragNormalOc = geomNormalOc[i];
        }
        fragColor    = geomColor[i];

        EmitVertex();
    }
    EndPrimitive();
}
