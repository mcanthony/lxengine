#version 150

uniform sampler2D unifTexture0;

in vec3    fragVertexOc;
in vec3    fragColor;

out vec4 outColor;

vec3 fog(vec3 color)
{
    float fogFactor = 1.0 - (gl_FragCoord.z / gl_FragCoord.w) / 400.0f;
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    return mix(color, vec3(0.05, 0.05, 0.07), 1.0 - fogFactor);
}

void main()
{	
    vec2 uv = fragColor.xy;
	outColor = texture2D(unifTexture0, uv);
    outColor.xyz = fog(outColor.xyz);
}
