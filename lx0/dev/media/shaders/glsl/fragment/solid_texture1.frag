#version 150

uniform sampler2D unifTexture0;

in vec3    fragVertexOc;
in vec3    fragColor;


out vec4 outColor;

void main()
{	
    vec2 uv = fragColor.xy;
	outColor = texture2D(unifTexture0, uv);
}
