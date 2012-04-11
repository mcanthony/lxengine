#version 150
#extension GL_ARB_explicit_attrib_location : enable

uniform	float	lineWidth = 0.75;
uniform vec4	lineColor = vec4(.5, .7, .96, 1.0);

in	vec3	fragPosition;
in 	vec3	fragNormal;
in  float	fragIntensity;
noperspective in vec3	fragEdgeDistance;

layout(location = 0) out vec4 outColor;

void main()
{
    // The shaded surface color.
	float i = fragIntensity;
	vec4 surfaceColor = vec4( .2, .65, .4, 1.0 ) * (i * .65 + .35);
	
	// Find the smallest distance
	float d = min( min( fragEdgeDistance.x, fragEdgeDistance.y ), fragEdgeDistance.z );
	
	// Determine the mix factor with the line color      
	float mixVal = smoothstep(lineWidth - 1, lineWidth + 1, d);
	
	// Mix the surface color with the line color
	vec4 edgeColor = mix(surfaceColor, lineColor, sqrt(i) * .5 + .05);
	outColor = mix(edgeColor, surfaceColor, mixVal);
}
