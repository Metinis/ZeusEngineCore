#version 450
#extension GL_GOOGLE_include_directive : require
#include "bindless.glsl"

#define PI 3.14159265359

const uint USE_ALBEDO   = 1u << 0u;
const uint USE_METALLIC = 1u << 1u;
const uint USE_ROUGHNESS= 1u << 2u;
const uint USE_NORMAL   = 1u << 3u;
const uint USE_AO       = 1u << 4u;

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inFragPos;
layout (location = 3) in vec2 inUV;
layout (location = 4) in flat uint inMat;
layout (location = 5) in mat3 inTBN;

layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform SceneData {
    mat4 proj;
    mat4 view;
    vec4 ambientColor;
    vec4 u_LightPos; // w for sun power
    vec4 sunlightColor;
    vec4 u_CameraPos;
} SceneDataBuffer;

layout( push_constant ) uniform PushConstants
{
    uint skyboxIdx;
    uint irradianceIdx;
    uint prefilterIdx;
    uint brdfTexIdx;
} pc;

float ggxDistribution(float nDotH, float roughness){
    // a = surface roucghness, when 0, smooth, when 1, rough
    // D = a^2 / PI * (dot(normal, half)^2 * (a^2-1) + 1)^2
    float alpha2 = roughness * roughness * roughness * roughness;
    float d = nDotH * nDotH * (alpha2 - 1) + 1;
    float ggxdistrib = alpha2 / (PI * d * d);
    return ggxdistrib;
}
float geomSmith(float dp, float roughness){
    // G = dot(normal, view) / (dot(normal, view)*(1 - K) + K)
    // K = ((a+1)^2) / 8
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float denom = dp * (1 - k) + k;
    return dp / denom;
}
vec3 schlickFresnel(float vDotH, vec3 F0, float roughness){
    //F = F0 + (1-F0)*(1-dot(view, half))^5
    //F0 = base reflectivity of the surface
    //vec3 F = F0 + (1 - F0) * pow(clamp(1.0 - vDotH, 0.0, 1.0), 5);
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - vDotH, 0.0, 1.0), 5.0);
    //return F;
}
//kd + ks = 1 always
//vec3 getNormalFromMapObjectSpace() {
    //vec3 N = texture(u_NormalMap, v_UV).rgb;
    //N = N * 2.0 - 1.0;// remap [0,1] -> [-1,1]
    //return normalize(N);
//}

void main()
{
    Material mat = materialBuffers.materials[inMat];
    vec3 albedo = mat.u_Albedo.rgb;
    vec3 normal = normalize(inNormal);
    float metallic = mat.u_Params.x;
    float roughness = mat.u_Params.y;;
    float ao = mat.u_Params.z;

    if((mat.flags & USE_ALBEDO) != 0u) {
        albedo = pow(sampleWithIdx(mat.albedoIndex, mat.samplerIndex, inUV).rgb, vec3(2.2));
    }

    if((mat.flags & USE_NORMAL) != 0u) {
        vec3 Nmap = sampleWithIdx(mat.normalIndex, mat.samplerIndex, inUV).rgb * 2.0 - 1.0;
        normal = normalize(inTBN * Nmap);
    }

    if((mat.flags & USE_METALLIC) != 0u) {
        metallic = sampleWithIdx(mat.metallicIndex, mat.samplerIndex, inUV).r;
    }

    if((mat.flags & USE_ROUGHNESS) != 0u) {
        roughness = sampleWithIdx(mat.roughnessIndex, mat.samplerIndex, inUV).r;
    }

    if((mat.flags & USE_AO) != 0u) {
        ao = sampleWithIdx(mat.aoIndex, mat.samplerIndex, inUV).r;
    }

    vec3 N = normal;
    //vec3 N = normalize(v_Normal);
    vec3 L = normalize(SceneDataBuffer.u_LightPos.xyz - inFragPos);
    vec3 V = normalize(SceneDataBuffer.u_CameraPos.xyz - inFragPos);
    vec3 H = normalize(L + V);

    float nDotL = max(dot(N, L), 0.0);
    float nDotV = max(dot(N, V), 0.0);
    float nDotH = max(dot(N, H), 0.0);
    float vDotH = max(dot(V, H), 0.0);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    /*if(u_Params.w > 0.0){
      F0 = albedo;
    }*/
    vec3 F = schlickFresnel(vDotH, F0, roughness);

    float G = geomSmith(nDotV, roughness) * geomSmith(nDotL, roughness);

    float D = ggxDistribution(nDotH, roughness);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * nDotV * nDotL + 0.001;
    vec3 specular = numerator / denominator;

    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kd * albedo / PI;

    vec3 R = reflect(-V, N);
    const float MAX_REFLECTION_LOD = 4.0;

    vec3 prefilteredColor = sampleWithIdxLod(pc.prefilterIdx, LINEAR_SAMPLER_CLAMPED_INDEX, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = sampleWithIdx(pc.brdfTexIdx, LINEAR_SAMPLER_CLAMPED_INDEX, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specularIBL = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 lightColor = vec3(1.0);

    vec3 Lo = (diffuse + specular) * lightColor * nDotL;

    vec3 irradiance = sampleWithIdx(pc.irradianceIdx, LINEAR_SAMPLER_CLAMPED_INDEX, N).rgb;
    vec3 diffuseIBL = irradiance * albedo;
    vec3 ambient = (diffuseIBL + specularIBL) * ao;

    vec3 color = Lo + ambient;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); //gamma correction

    outFragColor = vec4(color, 1.0);
}
