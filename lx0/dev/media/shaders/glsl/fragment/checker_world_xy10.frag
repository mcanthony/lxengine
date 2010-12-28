#version 150

uniform sampler2D unifTexture0;

in vec3    fragLightDirEc;

in vec3    fragVertexOc;
in vec3    fragNormalEc;
in vec3    fragNormalOc;

in vec3    fragColor;

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
    vec3 primary = fragColor; 
    
    vec2 uv = fract(fragVertexOc.xy / 10);
    vec3 secondary = texture2D(unifTexture0, uv).xyz;
    secondary = mix(secondary, vec3(1, 1, .8), .2 + fragColor.r);

    vec3 color = checker(fragVertexOc.xy / 10, primary, secondary);
    float diffuse = clamp( -dot(fragNormalOc, fragLightDirEc), 0.0, 1.0 );
    color *= (diffuse * .75 + .25);

    float fogFactor = 1.0 - (gl_FragCoord.z / gl_FragCoord.w) / 400.0f;
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    color = mix(color, vec3(0.05, 0.05, 0.07), 1.0 - fogFactor);

	outColor = vec4(color, 1.0);
}
