#version 150

vec2 uvCyclindrical (vec3 vertexOc, vec2 scale)
{
    return scale * (vertexOc.xy / length (vertexOc));
}  
        
vec3 checker(vec2 uv, vec3 primary, vec3 secondary)
{   
    vec2 t = abs( fract(uv) );
    ivec2 s = ivec2(trunc(2 * t));
      
    if ((s.x + s.y) % 2 == 0)
        return primary;
    else
        return secondary;
}

