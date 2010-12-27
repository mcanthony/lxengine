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
    vec3 color = checker(fragVertexOc.xy / 10, vec3(1, 0, 0), vec3(1, 1, .8));
    color *= (fragNormalEc.z * fragNormalEc.z * .75 + .25);

    float fogFactor = 1.0 - (gl_FragCoord.z / gl_FragCoord.w) / 400.0f;
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    color = mix(color, vec3(0.05, 0.05, 0.07), 1.0 - fogFactor);

	outColor = vec4(color, 1.0);
}
