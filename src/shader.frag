#version 330

in vec2 v_uv;
out vec4 fragColor;

uniform vec3 u_color;
uniform sampler2D u_texture;

void main(void)
{
	
	vec4 texture_color = texture2D(u_texture, v_uv);

	fragColor = vec4(texture_color.xyz, 1.0):
	
	// We're just going to paint the interpolated colour from the vertex shader
	//fragColor =  vec4(u_color, 1.0);
	//fragColor =  vec4(v_uv, 0.0, 1.0);
}
