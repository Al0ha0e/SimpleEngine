#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

layout(std140, binding = 0) uniform VPBlock{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};


uniform mat4 model;
//uniform uint pointlight_cnt;
//uniform uint spotlight_cnt;
//uniform uint directional_cnt;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}