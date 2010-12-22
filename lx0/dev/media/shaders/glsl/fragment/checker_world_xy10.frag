#version 150

varying in vec3    fragVertexOc;
varying in vec3    fragNormalEc;

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
    vec3 color = checker(fragVertexOc.xy / 100, vec3(1, 0, 0), vec3(1, 1, .6));

	outColor = vec4(color * (fragNormalEc.z * fragNormalEc.z * .75 + .25), 1.0);
}
