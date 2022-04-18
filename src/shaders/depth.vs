#version 450 core
layout (location = 0) in vec3 aPos;


layout(std140, binding = 0) uniform VPBlock{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
    vec4 camInfo;
};


uniform mat4 model;
//uniform uint pointlight_cnt;
//uniform uint spotlight_cnt;
//uniform uint directional_cnt;

void main()
{
    vec4 FragPos = model * vec4(aPos, 1.0);
    gl_Position = projection * view * FragPos;
}