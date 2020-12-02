#version 450

layout (location = 1) in vec3 a_pos;
layout (location = 2) in vec3 a_normal;
layout (location = 3) in vec2 a_uv;
layout (location = 4) in vec4 a_color;

layout (binding = 0) uniform UBO
{
	mat4 u_projection;
	mat4 u_model;
	mat4 u_view;
	vec3 u_lightpos;
};

layout (location = 0) out vec3 v_normal;
layout (location = 1) out vec2 v_uv;
layout (location = 2) out vec4 v_color;
layout (location = 3) out vec3 v_fragPos;
layout (location = 4) out vec3 v_lightDir;

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
