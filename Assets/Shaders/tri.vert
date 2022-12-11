#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout (set = 0, binding = 4) uniform TransformData
{
	mat4 modelViewProj;
} transformData[];

layout (push_constant) uniform PushConstants
{
	int textureIdx;
	int transformDataIdx;
} pushConstants;

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTexCoord;

layout (location = 0) out vec2 outUV;

void main()
{
	gl_Position = transformData[pushConstants.transformDataIdx].modelViewProj * vec4(vPos, 1.f);
	outUV = vTexCoord;
}