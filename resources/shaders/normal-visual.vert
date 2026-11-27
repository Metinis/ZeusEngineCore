#version 410 core

layout(std140) uniform View {
    mat4 u_VP;
};

layout(std140) uniform Instances {
    mat4 u_Model[100];
};

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec4 a_Color;

out DATA {
    mat4 v_VP;
    vec4 v_Color;
    vec2 v_UV;
    vec3 v_Normal;
    vec3 v_FragPos;
} data_out;


void main() {
    mat4 model = u_Model[gl_InstanceID];

    //Transform vertex position to clip space
    gl_Position = model * vec4(a_Position, 1.0);
    data_out.v_Color = a_Color;
    data_out.v_UV = a_TexCoord;
    data_out.v_Normal = mat3(transpose(inverse(model))) * a_Normal;
    data_out.v_VP = u_VP;
    data_out.v_FragPos = vec3(model * vec4(a_Position, 1.0));

}
