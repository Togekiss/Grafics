#version 330

in vec2 v_uv;
in vec3 v_normals;
in vec3 v_world_vertex_pos;
out vec4 fragColor;

uniform vec3 u_color;
uniform sampler2D u_texture;
uniform vec3 u_light_dir;
uniform vec3 u_light_color;
uniform vec3 u_cam_pos;
uniform float u_shininess;
uniform float u_ambient;

void main(void)

{

	vec3 texture_color = texture2D(u_texture, v_uv).xyz;
	vec3 ambient_color = texture_color * u_ambient;

	vec3 N = normalize(v_normals);
	vec3 L = normalize(u_light_dir);
	vec3 R = reflect(L, N);
	vec3 E = normalize(u_cam_pos - v_world_vertex_pos);
	float NdotL = max(dot(N, L), 0.0);
	float RdotE = pow( max(dot(R, -E), 0.0), u_shininess);


	vec3 diffuse_color = texture_color.xyz * NdotL;
	vec3 specular_color = texture_color * u_light_color * RdotE;

	vec3 final_color =  ambient_color + diffuse_color + specular_color;

	fragColor = vec4(final_color, 1.0);
	

	// We're just going to paint the interpolated colour from the vertex shader
	//fragColor =  vec4(u_color, 1.0);
	//fragColor =  vec4(v_uv, 0.0, 1.0);
}
