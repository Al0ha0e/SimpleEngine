#version 450 core

out vec4 FragColor;
in vec3 FragPos;  
in mat3 TBN;  
in vec2 TexCoords;

layout(std140, binding = 0) uniform VPBlock{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

layout(std140, binding = 1) uniform GIBlock{
    vec4 ambient;
};

uniform sampler2D diffuse;
uniform sampler2D specular;
uniform sampler2D normal;
uniform float shininess;

struct Light {
    vec4 position;  //w: intensity
    vec4 color;     //w: range
    vec4 direction; //w: spot angle
};


layout(std140, binding = 2) uniform point_block{
    Light pointlights[512];
};

layout(std140, binding = 3) uniform spot_block{
    Light spotlights[512];
};

layout(std140, binding = 4) uniform directional_block{
    Light directionals[8];
};


void handlePointLight(vec3 norm, vec3 viewDir, vec3 diffuse_rgb, vec3 specular_rgb){
    Light l0 = pointlights[0];

    // diffuse 
    vec3 lightDir = normalize(l0.position.xyz - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float intensity = l0.position.w; 
    vec3 f_diffuse = diff * diffuse_rgb;  
    
    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    //vec3 reflectDir = normalize(lightDir+viewDir);
    float spec = step(0.0,dot(lightDir, norm)) * pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    //float spec = pow(max(dot(norm, reflectDir), 0.0), shininess);
    vec3 f_specular = vec3(1.0, 1.0, 1.0) * spec * specular_rgb.r / 2.0;  
    
    float dist = length(l0.position.xyz - FragPos);
    float attenuation = 1.0 / (1 + 0.14 * dist + 0.07 * dist * dist);

    vec3 result = l0.color.xyz * attenuation * intensity * (f_diffuse + f_specular);
    FragColor += vec4(result, 1.0);
}


void handleSpotLight(vec3 norm, vec3 viewDir, vec3 diffuse_rgb, vec3 specular_rgb){
    Light l0 = spotlights[0];

    // diffuse 
    vec3 lightDir = normalize(l0.position.xyz - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float intensity = l0.position.w; 
    vec3 f_diffuse = diff * diffuse_rgb;  
    
    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    //vec3 reflectDir = normalize(lightDir+viewDir);
    float spec = step(0.0,dot(lightDir, norm)) * pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    //float spec = pow(max(dot(norm, reflectDir), 0.0), shininess);
    vec3 f_specular = vec3(1.0, 1.0, 1.0) * spec * specular_rgb.r / 2.0;  
    
    float dist = length(l0.position.xyz - FragPos);
    float attenuation = 1.0 / (1 + 0.14 * dist + 0.07 * dist * dist);
    float cutoff = l0.direction.w;
    float dirdet = dot(lightDir, normalize(-l0.direction.xyz));
    float diratten = 1.0 - clamp((cutoff - dirdet)/ (0.2 * cutoff),0.0,1.0);//sign(dirdet - cutoff);

    vec3 result = l0.color.xyz * diratten * attenuation * intensity * (f_diffuse + f_specular);
    FragColor += vec4(result, 1.0);
}

void handleDirectional(vec3 norm, vec3 viewDir, vec3 diffuse_rgb, vec3 specular_rgb){
    Light l0 = directionals[0];

    // diffuse 
    vec3 lightDir = normalize(-l0.direction.xyz);
    float diff = max(dot(norm, lightDir), 0.0);
    float intensity = l0.position.w; 
    vec3 f_diffuse = diff * diffuse_rgb;  
    
    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    //vec3 reflectDir = normalize(lightDir+viewDir);
    float spec = step(0.0,dot(lightDir, norm)) * pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    //float spec = pow(max(dot(norm, reflectDir), 0.0), shininess);
    vec3 f_specular = vec3(1.0, 1.0, 1.0) * spec * specular_rgb.r / 2.0;  
        
    vec3 result = l0.color.xyz * intensity * (f_diffuse + f_specular);
    FragColor += vec4(result, 1.0);
}

void main()
{
    // vec3 norm = TBN[2];
    vec3 norm = texture(normal,TexCoords).rgb;
    norm = normalize(norm * 2.0 - 1.0);   
    norm = normalize(TBN * norm);
    vec3 viewDir = normalize(viewPos.xyz - FragPos);
    vec3 diffuse_rgb = texture(diffuse, TexCoords).rgb;
    vec3 specular_rgb = texture(specular, TexCoords).rgb;
    vec3 f_ambient = ambient.xyz * diffuse_rgb;
    handlePointLight(norm,viewDir,diffuse_rgb,specular_rgb);
    handleSpotLight(norm,viewDir,diffuse_rgb,specular_rgb);
    handleDirectional(norm,viewDir,diffuse_rgb,specular_rgb);
    FragColor += vec4(f_ambient, 1.0);
}