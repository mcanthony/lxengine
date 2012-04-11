#version 150
#extension GL_ARB_explicit_attrib_location : enable

uniform		vec4	unifColor0 = vec4(22.0/255.0,79.0/255.0,182.0/255.0, 1.0);
uniform		vec4	unifColor1 = vec4(121.0/255.0, 98.0/255.0, 139.0/255.0, 1.0);

in 	vec3	fragNormal;
in	vec2	fragUV;
in  float	fragIntensity;

layout(location = 0) out vec4 outColor;

vec4 checker(vec2 uv, vec4 color0, vec4 color1)
{
	vec2 t = abs( fract(uv) );
    ivec2 s = ivec2(trunc(2 * t));

    if ((s.x + s.y) % 2 == 0)
		return color0;
	else
		return color1;
}

void main()
{
	outColor = checker(fragUV, unifColor0, unifColor1) * fragIntensity;
	outColor.a = 1.0;
}
