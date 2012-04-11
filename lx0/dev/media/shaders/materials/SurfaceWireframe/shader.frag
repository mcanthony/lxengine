#version 150
#extension GL_ARB_explicit_attrib_location : enable

uniform	float	lineWidth = 0.75;
uniform vec4	lineColor = vec4(.3, .5, .92, 1.0);

in	vec3	fragPosition;
in 	vec3	fragNormal;
in  float	fragIntensity;
noperspective in vec3	fragEdgeDistance;

layout(location = 0) out vec4 outColor;

void main()
{
    // The shaded surface color.
	float intensity = fragIntensity * .75 + .25;
	vec4 color = intensity * vec4( .2, .65, .4, 1.0 );
	
	// Find the smallest distance
	float d = min( min( fragEdgeDistance.x, fragEdgeDistance.y ), fragEdgeDistance.z );
	
	// Determine the mix factor with the line color      
	float mixVal = smoothstep(lineWidth - 1, lineWidth + 1, d) / 4.0 + .75;
	
	// Mix the surface color with the line color
	outColor = mix(lineColor, color, mixVal);
}
