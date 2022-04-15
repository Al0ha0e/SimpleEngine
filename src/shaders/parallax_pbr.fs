#version 450 core

#define max_point_light 512*8
#define max_spot_light 512*8

out vec4 FragColor;
in vec3 FragPos;  
in mat3 TBN;  
in vec2 TexCoords;

layout(std140, binding = 0) uniform VPBlock{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
    vec4 camInfo;
};

layout(std140, binding = 1) uniform GIBlock{
    vec4 ambient;
    vec2 screenSize;
};
layout (binding = 1) uniform sampler2D irradianceMap;
layout (binding = 2) uniform sampler2D prefilteredMap;
layout (binding = 3) uniform sampler2D lutMap;
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
    uint point_cnt;
};

layout(std140, binding = 3) uniform spot_block{
    Light spotlights[512];
    uint spot_cnt;
};

layout(std140, binding = 4) uniform directional_block{
    Light directionals[8];
    uint directional_cnt;
};

layout(std140, binding = 0) buffer LightIndexBlock{
    int point_index[max_point_light];
    int spot_index[max_spot_light];
    ivec2 light_index_pos;
};

layout(rgba32f, binding = 0) uniform image3D light_grid;

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

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
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


vec3 handlePointLight(vec3 N, vec3 V, vec3 albedo, vec2 material, ivec4 info)
{
    vec3 ret = vec3(0,0,0);
    for(int i = 0; i < info.y; i++){
        Light l0 = pointlights[point_index[info.x + i]];
        vec3 L = normalize(l0.position.xyz - FragPos);
        float intensity = l0.position.w;
        float dist = length(l0.position.xyz - FragPos);
        float attenuation = 1.0 / (1 + 0.14 * dist + 0.07 * dist * dist);
        vec3 radiance = l0.color.xyz * attenuation * intensity;
        ret += CalcPBR(N, V, L, radiance, albedo, material);
    }
    return ret;
}

vec3 handleSpotLight(vec3 N, vec3 V, vec3 albedo, vec2 material, ivec4 info)
{
    vec3 ret = vec3(0, 0, 0);
    for(uint i = 0; i < info.y; i++){
        Light l0 = spotlights[spot_index[info.x + i]];
        vec3 L = normalize(l0.position.xyz - FragPos);
        float intensity = l0.position.w;
        float dist = length(l0.position.xyz - FragPos);
        float attenuation = 1.0 / (1 + 0.14 * dist + 0.07 * dist * dist);
        float cutoff = l0.direction.w;
        float dirdet = dot(L, normalize(-l0.direction.xyz));
        float diratten = 1.0 - clamp((cutoff - dirdet)/ (0.2 * cutoff),0.0,1.0);
        vec3 radiance = l0.color.xyz * diratten * attenuation * intensity;
        ret += CalcPBR(N, V, L, radiance, albedo, material);
    }
    return ret;
}

vec3 handleDirectional(vec3 N, vec3 V, vec3 albedo, vec2 material)
{
    vec3 ret = vec3(0, 0, 0);
    for(uint i = 0; i < directional_cnt; i++){
        Light l0 = directionals[i];
        vec3 L = normalize(-l0.direction.xyz);
        float intensity = l0.position.w;
        vec3 radiance = l0.color.xyz * intensity;
        ret +=  CalcPBR(N, V, L, radiance, albedo, material);
    }
    return ret;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height =  texture(nhMap, texCoords).a;    
    vec2 p = viewDir.xy / (viewDir.z + 0.2) * (height - 0.5) * height_scale;
    return texCoords + p;    
}


const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    v = normalize(v);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
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
    
    float metallic = material.r;
    float roughness = material.g;
    float ao = material.b;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    vec3 kS = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness); 
    vec3 kD = 1.0 - kS;
    vec3 irradiance = texture(irradianceMap, SampleSphericalMap(N)).rgb;

    vec3 R = reflect(-V, N);
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilteredMap, SampleSphericalMap(R), roughness * MAX_REFLECTION_LOD).rgb; 
    vec2 envBRDF  = texture(lutMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

    vec3 color = (kD * irradiance * albedo + specular) * ao; //ambient.xyz * albedo;

    float near = camInfo.z;
    float far = camInfo.w;
    float width = screenSize.x;
    float height = screenSize.y;
    vec3 vpos = (view * vec4(FragPos, 1.0)).xyz;
    int zi = int(clamp(floor(16 * (-vpos.z - near) / (far - near)), 0, 15));
    int xi = int(clamp(floor(gl_FragCoord.x / width * 16), 0, 15));
    int yi = int(clamp(floor(gl_FragCoord.y / height * 16), 0, 15));
    ivec4 info = ivec4(imageLoad(light_grid, ivec3(xi,yi,zi)));

    color += handlePointLight(N, V, albedo, material.rg, info);
    color += handleSpotLight(N, V, albedo, material.rg, info);
    color += handleDirectional(N, V, albedo, material.rg);

    // HDR tonemapping
    //color = color / (color + vec3(1.0));
    // gamma correct
    //color = pow(color, vec3(1.0/2.2)); 
    FragColor = vec4(color, 1.0);
}