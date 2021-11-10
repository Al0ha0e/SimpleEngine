#version 450 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout(std140, binding = 0) uniform VPBlock{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

void main()
{
    TexCoords = aPos;
    gl_Position = projection * mat4(mat3(view)) * vec4(aPos, 1.0);
}