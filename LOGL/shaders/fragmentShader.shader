#version 430

out vec4 color;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

in vec4 varyingColor;

void main(void)
{
	color = varyingColor;
}

//#version 430
//
//uniform float offset;
//
//out vec4 color; 
//void main(void) 
//{ 
//	if (gl_FragCoord.x < 400) color = vec4(1.0 + offset, 0.0 + offset, 0.0 + offset, 1.0); else color = vec4(0.0 + offset, 0.0 + offset, 1.0 + offset, 1.0);
//	//color = vec4(1.0, 1.0, 0.0, 1.0);
//
//	/*if (gl_VertexID == 0)
//		color = vec4(1.0 + offset, 0.0 + offset, 0.0 + offset, 1);
//
//	if (gl_VertexID == 1)
//		color = vec4(0.0 + offset, 1.0 + offset, 0.0 + offset, 1);
//
//	if (gl_VertexID == 2)
//		color = vec4(0.0 + offset, 0.0 + offset, 1.0 + offset, 1);*/
//
//}