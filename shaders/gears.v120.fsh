#version 120

varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;
varying vec3 v_fragPos;
varying vec3 v_lightDir;

void main()
{
    float ambient = 0.1;
    float diff = max(dot(v_normal, v_lightDir), 0.0);
    vec4 finalColor = (ambient + diff) * v_color;
    gl_FragColor = vec4(finalColor.rgb, v_color.a);
}
