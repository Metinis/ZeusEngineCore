#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

#define LINEAR_SAMPLER_CLAMPED_INDEX 0
#define NEAREST_SAMPLER_CLAMPED_INDEX 1
#define LINEAR_SAMPLER_REPEAT_INDEX 2
#define NEAREST_SAMPLER_REPEAT_INDEX 3

layout(set = 1, binding = 0) uniform texture2D textures[];
layout(set = 1, binding = 0) uniform textureCube cubeTextures[];

layout(set = 1, binding = 1, rgba8) uniform image2D rgba8Images[];
layout(set = 1, binding = 1, rgba16f) uniform image2D rgba16fImages[];

layout(set = 1, binding = 1, rgba16f) uniform imageCube cubeImages[];

layout(set = 1, binding = 2) uniform sampler samplers[];

struct Material {
    vec4 u_Albedo;   // xyz = color
    vec4 u_Params;   // x=metallic, y=roughness, z=ao, w=unused

    uint albedoIndex;
    uint metallicIndex;
    uint roughnessIndex;
    uint normalIndex;
    uint aoIndex;

    uint samplerIndex;

    uint flags;
};

layout(std430, set = 2, binding = 0) readonly buffer MaterialBuffer {
    Material materials[];
} materialBuffers;

vec4 sampleWithIdx(uint textureIndex, uint samplerIdx, vec2 uv) {
    return texture(sampler2D(textures[nonuniformEXT(textureIndex)], samplers[nonuniformEXT(samplerIdx)]), uv);
}
vec4 sampleWithIdx(uint textureIndex, uint samplerIdx, vec3 dir) {
    return texture(samplerCube(cubeTextures[nonuniformEXT(textureIndex)], samplers[nonuniformEXT(samplerIdx)]), dir);
}
vec4 sampleWithIdxLod(uint textureIndex, uint samplerIdx, vec2 uv, float lod) {
    return textureLod(sampler2D(textures[nonuniformEXT(textureIndex)], samplers[nonuniformEXT(samplerIdx)]), uv, lod);
}
vec4 sampleWithIdxLod(uint textureIndex, uint samplerIdx, vec3 dir, float lod) {
    return textureLod(samplerCube(cubeTextures[nonuniformEXT(textureIndex)], samplers[nonuniformEXT(samplerIdx)]), dir, lod);
}