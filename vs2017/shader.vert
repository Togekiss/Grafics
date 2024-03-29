#version 330
 
in vec3 a_vertex;
in vec2 a_uv;
in vec3 a_normals;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat3 u_normal_matrix;

out vec3 v_world_vertex_pos;
out vec2 v_uv;
out vec3 v_normals;

void main()
{
	v_normals =  u_normal_matrix * a_normals;
	v_uv = a_uv;
	// position of the vertex
	gl_Position = u_projection * u_view * u_model * vec4( a_vertex , 1.0 );
	v_world_vertex_pos = (u_model * vec4(a_vertex, 1.0)).xyz;

}

