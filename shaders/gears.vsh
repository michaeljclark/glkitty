#version 140

attribute vec3 a_pos;
attribute vec3 a_normal;
attribute vec2 a_uv;
attribute vec4 a_color;

uniform mat4 u_projection;
uniform mat4 u_model;
uniform mat4 u_view;
uniform vec3 u_lightpos;

out vec3 v_normal;
out vec2 v_uv;
out vec4 v_color;
out vec3 v_fragPos;
out vec3 v_lightDir;

void main()
{
	mat4 modelView = u_view * u_model;
	vec4 pos = modelView * vec4(a_pos,1.0);
	mat3 normalMatrix = transpose(inverse(mat3(modelView)));

	v_normal = normalize(normalMatrix * a_normal);
	v_uv = a_uv;
	v_color = a_color;
	v_fragPos = vec3(u_model * vec4(a_pos,1.0));
	v_lightDir = normalize(u_lightpos - v_fragPos);

	gl_Position = u_projection * pos;
}