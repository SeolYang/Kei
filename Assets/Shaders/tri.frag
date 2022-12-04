#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout (push_constant) uniform PushConstants
{
	int colorDataIndex;
	int textureIndex;
} pushConstants;

layout (set = 0, binding = 2) uniform sampler2D textures[];

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = vec4(texture(textures[pushConstants.textureIndex], inUV).rgb, 1.f);
}