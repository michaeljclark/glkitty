#version 150

in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;
in vec3 v_fragPos;
in vec3 v_lightDir;

out vec4 outFragColor;

void main()
{
    float ambient = 0.1;
    float diff = max(dot(v_normal, v_lightDir), 0.0);
    vec4 finalColor = (ambient + diff) * v_color;
    outFragColor = vec4(finalColor.rgb, v_color.a);
}
