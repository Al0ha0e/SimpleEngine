#version 450 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D equirectangularMap;

const float PI = 3.14159265359;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    v = normalize(v);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec3 GetDir(vec2 uv)
{
    uv -= vec2(0.5, 0.5);
    uv *= vec2(2*PI, PI);
    vec3 v;
    v.x = cos(uv.y) * cos(uv.x);
    v.y = sin(uv.y);
    v.z = cos(uv.y) * sin(uv.x);
    return normalize(v);
}

void main()
{	
    vec3 N = GetDir(TexCoords);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.04;//0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(equirectangularMap, SampleSphericalMap(sampleVec)).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    FragColor = vec4(irradiance, 1.0);
}