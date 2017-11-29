#version 120
 
attribute vec3 a_vertex;
attribute vec3 a_color;

varying vec3 v_color;

void main()
{
	// position of the vertex
	gl_Position = vec4( a_vertex , 1.0 );

	// pass the colour to the fragment shader
	v_color = a_color;
}

