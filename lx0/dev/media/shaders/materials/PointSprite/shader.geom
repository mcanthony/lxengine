#version 150 
#extension GL_EXT_geometry_shader4 : enable

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 	unifProjMatrix;

out vec2 fragUV;

void main()
{
	const float Size2 = .25;

	gl_Position = unifProjMatrix * (vec4(-Size2,-Size2,0.0,0.0) + gl_in[0].gl_Position);
	fragUV = vec2(0.0,0.0);
	EmitVertex();
	
	gl_Position = unifProjMatrix * (vec4( Size2,-Size2,0.0,0.0) + gl_in[0].gl_Position);
	fragUV = vec2(1.0,0.0);
	EmitVertex();
	
	gl_Position = unifProjMatrix * (vec4(-Size2, Size2,0.0,0.0) + gl_in[0].gl_Position);
	fragUV = vec2(0.0,1.0);
	EmitVertex();
	
	gl_Position = unifProjMatrix * (vec4( Size2, Size2,0.0,0.0) + gl_in[0].gl_Position);
	fragUV = vec2(1.0,1.0);
	EmitVertex();
	
	EndPrimitive();
}