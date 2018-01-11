#version 330

in vec2 v_uv;
out vec4 fragColor;

uniform vec3 u_color;
uniform sampler2D u_texture;
uniform float u_transparency;

void main(void)

{

	vec4 texture_color = texture2D(u_texture, v_uv);

	vec3 final_color = texture_color.xyz;
	fragColor = vec4(final_color, u_transparency);

}
