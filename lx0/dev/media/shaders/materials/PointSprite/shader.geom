#version 150 
#extension GL_EXT_geometry_shader4 : enable

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 	unifProjMatrix;
uniform float   unifSize;

out vec2 fragUV;

void main()
{
	gl_Position = unifProjMatrix * (vec4(-unifSize,-unifSize,0.0,0.0) + gl_in[0].gl_Position);
	fragUV = vec2(0.0,0.0);
	EmitVertex();
	
	gl_Position = unifProjMatrix * (vec4( unifSize,-unifSize,0.0,0.0) + gl_in[0].gl_Position);
	fragUV = vec2(1.0,0.0);
	EmitVertex();
	
	gl_Position = unifProjMatrix * (vec4(-unifSize, unifSize,0.0,0.0) + gl_in[0].gl_Position);
	fragUV = vec2(0.0,1.0);
	EmitVertex();
	
	gl_Position = unifProjMatrix * (vec4( unifSize, unifSize,0.0,0.0) + gl_in[0].gl_Position);
	fragUV = vec2(1.0,1.0);
	EmitVertex();
	
	EndPrimitive();
}