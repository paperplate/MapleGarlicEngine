#version 460

layout(location = 0) in vec4 fragColor;
layout(location = 0) out vec4 outColor;

layout(std140, set = 3, binding = 0) uniform UniformBlock {
  float time;
};

void main() {
  float pulse = sin(time * 2.0) * 0.5 + 0.5;
  outColor = vec4(fragColor.rgb * (0.8 + pulse * 0.5), fragColor.a);
}
