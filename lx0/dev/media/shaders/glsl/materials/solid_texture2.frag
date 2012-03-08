#version 150

uniform sampler2D unifTexture0;

in  vec2 fragUV;
out vec4 outColor;

void main()
{	   
	vec4 texColor = texture2D(unifTexture0, fragUV);
    //if (outColor.a < .01)
    //discard;
    outColor = vec4(1.0 - texColor.r, texColor.b, 1.0 - texColor.g, 1.0);
}
