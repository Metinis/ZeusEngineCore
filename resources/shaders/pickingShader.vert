#version 410 core
layout(location = 0) in vec3 a_Position;
layout(std140) uniform View {
  mat4 u_VP;
};
layout(std140) uniform Instances {
  mat4 u_Model[100];
};
void main() {
    mat4 model = u_Model[gl_InstanceID];
    gl_Position = u_VP * model * vec4(a_Position, 1.0);
}
