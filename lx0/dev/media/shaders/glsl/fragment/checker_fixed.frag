uniform vec3 uniCheckerPrimaryColor;     
uniform vec3 uniCheckerSecondaryColor;

in vec3         fragVertexOc;
in vec3         fragVertexEc;
in vec3         fragNormalOc;
in vec3         fragNormalEc;
in vec3         fragColor;

function uvCyclindrical (vec3 vertexOc)
{
    return scale * (vertexOc.xy / length (vertexOc));
}  
        
function checker(vec2 uv, vec3 primary, vec3 secondary)
{   
    vec2 t = abs( fract(uv) );
    ivec2 s = ivec2(trunc(2 * t));
      
    if ((s.x + s.y) % 2 == 0)
        return primary;
    else
        return secondary;
}

void main(void)
{
    gl_FragColor = vec4( 
        checker( 
            uvCyclindrical(
                fragVertexOc
            ),
            uniCheckerPrimaryColor,
            uniCheckerSecondaryColor
        ),
        1.0
    );
}
