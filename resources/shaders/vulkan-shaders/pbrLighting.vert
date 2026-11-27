#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outFragPos;
layout (location = 3) out vec2 outUV;
layout (location = 4) out flat uint outMat;
layout (location = 5) out mat3 outTBN;

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

    outFragPos = vec3(ObjectDataBuffer.objects[gl_InstanceIndex].matrix * vec4(v.position, 1.0f));
    gl_Position = SceneDataBuffer.proj * SceneDataBuffer.view * ObjectDataBuffer.objects[gl_InstanceIndex].matrix * vec4(v.position, 1.0f);

    mat3 normalMatrix = mat3(transpose(inverse(ObjectDataBuffer.objects[gl_InstanceIndex].matrix)));
    outNormal = normalMatrix * v.Normal;

    outMat = ObjectDataBuffer.objects[gl_InstanceIndex].matIndex;
    outUV = v.TexCoords;

    vec3 T = normalize(normalMatrix * v.Tangent);
    vec3 N = normalize(normalMatrix * v.Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = mat3(T, B, N);
    outTBN = TBN;

    outColor = vec3(1.0, 1.0, 1.0);
}
