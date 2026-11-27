#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 outUV;

struct Vertex {
    vec3 position;
    float _pad0;
    vec3 Normal;
    float _pad1;
    vec2 TexCoords;
    float _pad2;
    float _pad3;
    vec4 Color;
    vec3 Tangent;
    float _pad4;
    vec3 Bitangent;
    float _pad5;
};

layout(set = 0, binding = 0) uniform SceneData {
    mat4 proj;
    mat4 view;
    vec4 ambientColor;
    vec4 u_LightPos; // w for sun power
    vec4 sunlightColor;
} SceneDataBuffer;

layout(buffer_reference, std430) readonly buffer VertexBufferAdr {
    Vertex vertices[];
};

struct Object {
    uint matIndex;
    mat4 matrix;
    VertexBufferAdr vertexBufferAdr;
};
layout(set = 0, binding = 1) readonly buffer ObjectData {
    Object objects[];
} ObjectDataBuffer;

void main()
{
    Vertex v = ObjectDataBuffer.objects[gl_InstanceIndex].vertexBufferAdr.vertices[gl_VertexIndex];

    outUV = v.position;
    vec4 pos = SceneDataBuffer.proj * mat4(mat3(SceneDataBuffer.view)) * vec4(v.position, 1.0);

    gl_Position = pos.xyww;
}
