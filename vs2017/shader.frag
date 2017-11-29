#version 330

in vec2 v_uv;
in vec3 v_normals;
out vec4 fragColor;

uniform vec3 u_color;
uniform sampler2D u_texture;
uniform vec3 u_light_dir;

void main(void)

{

	vec4 texture_color = texture2D(u_texture, v_uv);

	vec3 N = normalize(v_normals);
	vec3 L = normalize(u_light_dir);
	float NdotL = max(dot(N, L), 0.0);

	vec3 final_color = texture_color.xyz * NdotL;
	fragColor = vec4(final_color, 1.0);


	// We're just going to paint the interpolated colour from the vertex shader
	//fragColor =  vec4(u_color, 1.0);
	//fragColor =  vec4(v_uv, 0.0, 1.0);
}
