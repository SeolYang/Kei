#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout (set = 0, binding = 4) uniform ColorData
{
	vec4 colors[3];
} colorData[];

layout (push_constant) uniform PushConstants
{
	int colorDataIndex;
} pushConstants;

layout (location=0) out vec4 outColor;

void main()
{
	const vec3 positions[3] = vec3[3](
		vec3(1.f, 1.f, 0.0f),
		vec3(-1.f, 1.f, 0.0f),
		vec3(0.f, -1.0f, 0.0f)
	);

	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
	outColor = colorData[pushConstants.colorDataIndex].colors[gl_VertexIndex];
}