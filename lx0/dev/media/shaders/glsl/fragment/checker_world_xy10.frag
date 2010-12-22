#version 150

varying in vec3    fragVertexOc;

out vec4 outColor;

vec3 checker(vec2 uv, vec3 primary, vec3 secondary)
{   
    vec2 t = abs( fract(uv) );
    ivec2 s = ivec2(trunc(2 * t));
      
    if ((s.x + s.y) % 2 == 0)
        return primary;
    else
        return secondary;
}

void main()
{	
	outColor = vec4(checker(fragVertexOc.xy / 100, vec3(1, 0, 0), vec3(1, 1, .6)), 1.0);
}
