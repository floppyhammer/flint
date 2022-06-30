#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform samplerCube uCubemap;

void main() {
    outColor = texture(uCubemap, inTexCoord);
}
