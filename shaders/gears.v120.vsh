#version 120

attribute vec3 a_pos;
attribute vec3 a_normal;
attribute vec2 a_uv;
attribute vec4 a_color;

uniform mat4 u_projection;
uniform mat4 u_model;
uniform mat4 u_view;
uniform vec3 u_lightpos;

varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;
varying vec3 v_fragPos;
varying vec3 v_lightDir;

float det(mat2 matrix) {
    return matrix[0].x * matrix[1].y - matrix[0].y * matrix[1].x;
}

mat3 inverse(mat3 matrix) {
    vec3 row0 = matrix[0];
    vec3 row1 = matrix[1];
    vec3 row2 = matrix[2];

    vec3 minors0 = vec3(
        det(mat2(row1.y, row1.z, row2.y, row2.z)),
        det(mat2(row1.z, row1.x, row2.z, row2.x)),
        det(mat2(row1.x, row1.y, row2.x, row2.y))
    );
    vec3 minors1 = vec3(
        det(mat2(row2.y, row2.z, row0.y, row0.z)),
        det(mat2(row2.z, row2.x, row0.z, row0.x)),
        det(mat2(row2.x, row2.y, row0.x, row0.y))
    );
    vec3 minors2 = vec3(
        det(mat2(row0.y, row0.z, row1.y, row1.z)),
        det(mat2(row0.z, row0.x, row1.z, row1.x)),
        det(mat2(row0.x, row0.y, row1.x, row1.y))
    );

    mat3 adj = transpose(mat3(minors0, minors1, minors2));

    return (1.0 / dot(row0, minors0)) * adj;
}

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