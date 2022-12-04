#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout (set = 0, binding = 4) uniform ColorData
{
	vec4 colors[3];
} colorData[];

layout (push_constant) uniform PushConstants
{
	int colorDataIndex;
	int textureIndex;
} pushConstants;

layout (location = 0) in vec4 vPos;
layout (location = 1) in vec2 vTexCoord;

layout (location = 0) out vec2 outUV;

void main()
{
	gl_Position = vPos;
	outUV = vTexCoord;
}