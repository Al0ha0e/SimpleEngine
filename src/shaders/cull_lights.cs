#version 450 core

#define max_point_light 65536
#define max_spot_light 65536
#define max_local_cnt 64

layout (local_size_x=8,local_size_y=8,local_size_z=16) in;

layout(std140, binding = 0) uniform VPBlock{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
    vec4 camInfo;
};

struct Light {
    vec4 position;  //w: intensity
    vec4 color;     //w: range
    vec4 direction; //w: spot angle
};


layout(std140, binding = 2) uniform point_block{
    Light pointlights[512];
    int point_cnt;
};

layout(std140, binding = 3) uniform spot_block{
    Light spotlights[512];
    int spot_cnt;
};

layout(std430, binding = 0) buffer LightIndexBlock{
    int point_index[max_point_light];
    int spot_index[max_spot_light];
    ivec2 light_index_pos;
};

layout(rgba32f, binding = 0) uniform image3D light_grid;

float test(vec3 minn, vec3 maxx, vec3 center, float r ){
    center = (view * vec4(center, 1.0)).xyz;
    float dist = 0;
    float f,v;
    for(int i = 0; i < 3; ++i){
        v = center[i];
        f = minn[i] - min(minn[i], v);
        dist += f * f;
        f = max(maxx[i], v) - maxx[i];
        dist += f * f;
    }
    return float(dist <= r * r);
}

void main()
{
    int point_idxs[max_local_cnt];
    int spot_idxs[max_local_cnt];
    float fov = camInfo.x;
    float aspect = camInfo.y;
    float near = camInfo.z;
    float far = camInfo.w;

    vec3 minn, maxx;
    minn.z = -(near + (far - near) * (gl_GlobalInvocationID.z + 1) / 16);
    maxx.z = minn.z + (far - near) / 16;
    float h1 = -minn.z * tan(fov / 2);
    float h2 = -maxx.z * tan(fov / 2);
    minn.y = min(
        min(-h1 + gl_GlobalInvocationID.y * h1 / 4, -h2 + gl_GlobalInvocationID.y * h2 / 4),
        min(-h1 + (gl_GlobalInvocationID.y + 1) * h1 / 4, -h2 + (gl_GlobalInvocationID.y + 1) * h2 / 4));
    maxx.y = max(
        max(-h1 + gl_GlobalInvocationID.y * h1 / 4, -h2 + gl_GlobalInvocationID.y * h2 / 4),
        max(-h1 + (gl_GlobalInvocationID.y + 1) * h1 / 4, -h2 + (gl_GlobalInvocationID.y + 1) * h2 / 4));
    float w1 = h1 * aspect;
    float w2 = h2 * aspect;
    minn.x = min(
        min(-w1 + gl_GlobalInvocationID.x * w1 / 4, -w2 + gl_GlobalInvocationID.x * w2 / 4),
        min(-w1 + (gl_GlobalInvocationID.x + 1) * w1 / 4, -w2 + (gl_GlobalInvocationID.x + 1) * w2 / 4));
    maxx.x = max(
        max(-w1 + gl_GlobalInvocationID.x * w1 / 4, -w2 + gl_GlobalInvocationID.x * w2 / 4),
        max(-w1 + (gl_GlobalInvocationID.x + 1) * w1 / 4, -w2 + (gl_GlobalInvocationID.x + 1) * w2 / 4));

    int local_point_cnt = 0;
    int local_spot_cnt = 0;

    for(int i = 0; i < point_cnt; i++){
        Light light = pointlights[i];

        float intensity = light.position.w;
        float c = (1.0 - 5.0 * intensity);
        float r = -1 + sqrt(max(0.14*0.14 - 4 * 0.07 * c, 0)) / 0.14;

        if(test(minn, maxx, light.position.xyz, r) > 0.5){
            point_idxs[local_point_cnt++] = i;
            if(local_point_cnt == max_local_cnt) break;
        }
    }   

    for(int i = 0; i < spot_cnt; i++){
        Light light = spotlights[i];
        float intensity = light.position.w;
        float c = (1.0 - 5.0 * intensity);
        float r = -1.0 + sqrt(0.14*0.14 + 4 * 0.07 * c) / 0.14;
        if(test(minn, maxx, light.position.xyz, r) > 0.5){
            spot_idxs[local_spot_cnt++] = i;
            if(local_spot_cnt == max_local_cnt) break;
        }
    }

    int point_st = atomicAdd(light_index_pos.x, local_point_cnt);
    int spot_st = atomicAdd(light_index_pos.y, local_spot_cnt);

    imageStore(
        light_grid, 
        ivec3(gl_GlobalInvocationID), 
        vec4(point_st, local_point_cnt, spot_st, local_spot_cnt));

    for(int i = 0; i < local_point_cnt; i++)
        point_index[point_st + i] = point_idxs[i];
    for(int i = 0; i < local_spot_cnt; i++)
        spot_index[spot_st + i] = spot_idxs[i];
}