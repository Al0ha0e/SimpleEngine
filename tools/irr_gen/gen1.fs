#version 450 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D equirectangularMap;

void main()
{   
    vec3 color = texture(equirectangularMap, TexCoords).rgb;
    FragColor = vec4(color, 1.0);
}
