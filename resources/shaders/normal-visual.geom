#version 410 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

out vec4 v_Color;
out vec2 v_UV;
out vec3 v_Normal;
out vec3 v_FragPos;

in DATA {
    mat4 v_VP;
    vec4 v_Color;
    vec2 v_UV;
    vec3 v_Normal;
    vec3 v_FragPos;
} data_in[];

void main(){
    gl_Position = data_in[0].v_VP * gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = data_in[0].v_VP * (gl_in[0].gl_Position + 0.1f * vec4(data_in[0].v_Normal, 0.0));
    EmitVertex();
    EndPrimitive();

    gl_Position = data_in[1].v_VP * gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = data_in[1].v_VP * (gl_in[1].gl_Position + 0.1f * vec4(data_in[1].v_Normal, 0.0));
    EmitVertex();
    EndPrimitive();

    gl_Position = data_in[2].v_VP * gl_in[2].gl_Position;
    EmitVertex();
    gl_Position = data_in[2].v_VP * (gl_in[2].gl_Position + 0.1f * vec4(data_in[2].v_Normal, 0.0));
    EmitVertex();
    EndPrimitive();
}