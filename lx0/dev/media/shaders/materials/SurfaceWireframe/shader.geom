#version 150 
#extension GL_EXT_geometry_shader4 : enable

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform	mat4	unifViewportMatrix;

in 		vec3	geomNormal[];
in		vec3	geomPosition[];		
in 		float 	geomIntensity[];

out					vec3	fragNormal;
out					vec3	fragPosition;
out					float	fragIntensity;
noperspective out	vec3	fragEdgeDistance;


void main()
{
	//
	// Transform the vertices into viewport space
	//
	vec3 p0 = vec3(unifViewportMatrix * (gl_in[0].gl_Position /	gl_in[0].gl_Position.w));
	vec3 p1 = vec3(unifViewportMatrix * (gl_in[1].gl_Position / gl_in[1].gl_Position.w));
	vec3 p2 = vec3(unifViewportMatrix * (gl_in[2].gl_Position / gl_in[2].gl_Position.w));
	
	//
	// Find the altitudes (ha, hb and hc)
	//
	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);
	float alpha = acos( (b*b + c*c - a*a) / (2.0*b*c) );
	float beta = acos( (a*a + c*c - b*b) / (2.0*a*c) );
	float ha = abs( c * sin( beta ) );
	float hb = abs( c * sin( alpha ) );
	float hc = abs( b * sin( alpha ) );
	
	//
	// Send the triangle along with the edge distances
	//
	fragEdgeDistance = vec3( ha, 0, 0 );
	fragNormal = geomNormal[0];
	fragPosition = geomPosition[0];
	fragIntensity = geomIntensity[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	
	fragEdgeDistance = vec3( 0, hb, 0 );
	fragNormal = geomNormal[1];
	fragPosition = geomPosition[1];
	fragIntensity = geomIntensity[1];
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	
	fragEdgeDistance = vec3( 0, 0, hc );
	fragNormal = geomNormal[2];
	fragPosition = geomPosition[2];
	fragIntensity = geomIntensity[2];
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	
	EndPrimitive();
}
