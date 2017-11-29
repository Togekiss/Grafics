#version 120


varying vec3 v_color;

void main(void)
{
	// We're just going to paint the interpolated colour from the vertex shader
	gl_FragColor =  vec4(v_color, 1.0);
}
