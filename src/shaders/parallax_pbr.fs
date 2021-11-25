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

uniform sampler2D albedoMap;
uniform sampler2D mraMap;
uniform sampler2D nhMap;
uniform float height_scale;

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

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 CalcPBR(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 albedo, vec2 material)
{
    float metallic = material.r;
    float roughness = material.g;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 numerator    = NDF * G * F; 
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // add to outgoing radiance Lo
    return (kD * albedo / PI + specular) * radiance * NdotL;
}


vec3 handlePointLight(vec3 N, vec3 V, vec3 albedo, vec2 material)
{
    Light l0 = pointlights[0];
    vec3 L = normalize(l0.position.xyz - FragPos);
    float intensity = l0.position.w;
    float dist = length(l0.position.xyz - FragPos);
    float attenuation = 1.0 / (1 + 0.14 * dist + 0.07 * dist * dist);
    vec3 radiance = l0.color.xyz * attenuation * intensity;
    return CalcPBR(N, V, L, radiance, albedo, material);
}

vec3 handleSpotLight(vec3 N, vec3 V, vec3 albedo, vec2 material)
{
    Light l0 = spotlights[0];
    vec3 L = normalize(l0.position.xyz - FragPos);
    float intensity = l0.position.w;
    float dist = length(l0.position.xyz - FragPos);
    float attenuation = 1.0 / (1 + 0.14 * dist + 0.07 * dist * dist);
    float cutoff = l0.direction.w;
    float dirdet = dot(L, normalize(-l0.direction.xyz));
    float diratten = 1.0 - clamp((cutoff - dirdet)/ (0.2 * cutoff),0.0,1.0);
    vec3 radiance = l0.color.xyz * diratten * attenuation * intensity;
    return CalcPBR(N, V, L, radiance, albedo, material);
}

vec3 handleDirectional(vec3 N, vec3 V, vec3 albedo, vec2 material)
{
    Light l0 = directionals[0];
    vec3 L = normalize(-l0.direction.xyz);
    float intensity = l0.position.w;
    vec3 radiance = l0.color.xyz * intensity;
    return CalcPBR(N, V, L, radiance, albedo, material);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height =  texture(nhMap, texCoords).a;    
    vec2 p = viewDir.xy / (viewDir.z + 0.2) * (height * height_scale  - height_scale * 0.5);
    return texCoords + p;    
}

void main()
{
    vec3 V = normalize(viewPos.xyz - FragPos);
    vec2 texCoords = ParallaxMapping(TexCoords, normalize(transpose(TBN) * V));

    vec3 N = texture(nhMap, texCoords).rgb;
    N = normalize(N * 2.0 - 1.0);   
    N = normalize(TBN * N);
    vec3 albedo = texture(albedoMap, texCoords).rgb;
    vec3 material = texture(mraMap, texCoords).rgb;
    
    vec3 color = ambient.rgb * albedo * material.b;

    color += handlePointLight(N, V, albedo, material.rg);
    color += handleSpotLight(N, V, albedo, material.rg);
    color += handleDirectional(N, V, albedo, material.rg);

    // HDR tonemapping
    //color = color / (color + vec3(1.0));
    // gamma correct
    //color = pow(color, vec3(1.0/2.2)); 
    FragColor = vec4(color, 1.0);
}