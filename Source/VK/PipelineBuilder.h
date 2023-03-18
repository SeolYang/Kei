#pragma once
#include <PCH.h>

namespace sy::vk
{
class ShaderModule;
class VertexInputBuilder;
class GraphicsPipelineBuilder
{
public:
    GraphicsPipelineBuilder();

    GraphicsPipelineBuilder& SetDefault();

    GraphicsPipelineBuilder& SetPipelineCreateFlags(VkPipelineCreateFlags flags);

    GraphicsPipelineBuilder& AddShaderStage(const ShaderModule& shaderModule);

    GraphicsPipelineBuilder& SetVertexInputLayout(const VertexInputBuilder& builder);

    /** Input Assembly State */

    GraphicsPipelineBuilder& SetPrimitiveTopology(VkPrimitiveTopology topology);
    GraphicsPipelineBuilder& SetPrimitiveRestart(bool bPrimitiveRestartEnable);

    /** Tessellation State */

    GraphicsPipelineBuilder& SetPatchControlPoints(uint32_t patchControlPoints);

    /** Rasterization State */

    GraphicsPipelineBuilder& AddViewport(VkViewport viewport);
    GraphicsPipelineBuilder& AddViewport(float x, float y, float width, float height, float minDepth, float maxDepth);

    GraphicsPipelineBuilder& AddScissor(VkRect2D scissor);
    GraphicsPipelineBuilder& AddScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

    GraphicsPipelineBuilder& SetDepthClampEnable(bool bDepthClampEnable);
    GraphicsPipelineBuilder& SetRasterizerDiscardEnable(bool bRasterizerDiscardEnable);
    GraphicsPipelineBuilder& SetPolygonMode(VkPolygonMode polygonMode);
    GraphicsPipelineBuilder& SetCullMode(VkCullModeFlags cullMode);
    GraphicsPipelineBuilder& SetFrontFace(VkFrontFace frontFace);
    GraphicsPipelineBuilder& SetDepthBiasEnable(bool bDepthBiasEnable);
    GraphicsPipelineBuilder& SetDepthBiasConstantFactor(float depthBiasConstantFactor);
    GraphicsPipelineBuilder& SetDepthBiasClamp(float depthBiasClamp);
    GraphicsPipelineBuilder& SetDepthBiasSlopeFactor(float depthBiasSlopeFactor);
    GraphicsPipelineBuilder& SetLineWidth(float lineWidth);

    /** Multisample State */

    GraphicsPipelineBuilder& SetRasterizerSamples(VkSampleCountFlagBits rasterizerSamples);
    GraphicsPipelineBuilder& SetSampleShadingEnable(bool bSampleShadingEnable);
    GraphicsPipelineBuilder& SetMinSampleShading(float minSampleShading);
    GraphicsPipelineBuilder& SetSampleMask(uint32_t* sampleMask);
    GraphicsPipelineBuilder& SetAlphaToCoverageEnable(bool bAlphaToCoverageEnable);
    GraphicsPipelineBuilder& SetAlphaToOneEnable(bool bAlphaToOne);

    /** Depth-Stencil State */

    GraphicsPipelineBuilder& SetDepthTestEnable(bool bDepthTestEnable);
    GraphicsPipelineBuilder& SetDepthWriteEnable(bool bDepthWriteEnable);
    GraphicsPipelineBuilder& SetDepthCompareOp(VkCompareOp depthCompareOp);
    GraphicsPipelineBuilder& SetDepthBoundsTestEnable(bool bDepthBoundsTestEnable);

    GraphicsPipelineBuilder& SetStencilTestEnable(bool bStencilTestEnable);
    GraphicsPipelineBuilder& SetStencilOpStateFront(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp, uint32_t compareMask, uint32_t writeMask, uint32_t reference);
    GraphicsPipelineBuilder& SetStencilOpStateBack(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp, uint32_t compareMask, uint32_t writeMask, uint32_t reference);
    GraphicsPipelineBuilder& SetMinDepthBounds(float minDepthBounds);
    GraphicsPipelineBuilder& SetMaxDepthBounds(float maxDepthBounds);

    /** Color Blend Attachment State */

    GraphicsPipelineBuilder& AddColorBlendAttachmentState(bool bBlendEnable, VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorBlendOp, VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp, VkColorComponentFlags colorWriteMask);
    GraphicsPipelineBuilder& DefaultColorBlendAttachment();
    GraphicsPipelineBuilder& SetColorBlendLogicOpEnable(bool bLogicOpEnable);
    GraphicsPipelineBuilder& SetColorBlendLogicOp(VkLogicOp logicOp);
    GraphicsPipelineBuilder& SetColorBlendConstants(std::array<float, 4> blendConstants);

    GraphicsPipelineBuilder& SetPipelineLayout(VkPipelineLayout pipelineLayout);

    VkPipelineLayout GetLayout() const
    {
        return pipelineLayout;
    }

    GraphicsPipelineBuilder& SetDepthStencilFormat(VkFormat format = VK_FORMAT_D24_UNORM_S8_UINT);

    [[nodiscard]] VkGraphicsPipelineCreateInfo Build() const;

private:
    VkPipelineCreateFlags                            pipelineCreateFlags;
    std::vector<VkPipelineShaderStageCreateInfo>     shaderStages;
    VkPipelineInputAssemblyStateCreateInfo           inputAssembly;
    VkPipelineTessellationStateCreateInfo            tessellation;
    VkPipelineViewportStateCreateInfo                viewportState;
    std::vector<VkViewport>                          viewports;
    std::vector<VkRect2D>                            scissors;
    VkPipelineRasterizationStateCreateInfo           rasterizer;
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    VkPipelineColorBlendStateCreateInfo              colorBlend;
    VkPipelineMultisampleStateCreateInfo             multiSampling;
    VkPipelineLayout                                 pipelineLayout;
    VkPipelineDepthStencilStateCreateInfo            depthStencil;

    VkPipelineRenderingCreateInfo renderingCreateInfo;

    std::optional<VkPipelineVertexInputStateCreateInfo> vertexInput;
};

class ComputePipelineBuilder
{
public:
    ComputePipelineBuilder();

    ComputePipelineBuilder& SetDefault();
    ComputePipelineBuilder& SetCreateFlags(VkPipelineCreateFlags flags);
    ComputePipelineBuilder& SetShader(const ShaderModule& shaderModule);
    ComputePipelineBuilder& SetPipelineLayout(VkPipelineLayout layout);

    VkComputePipelineCreateInfo Build() const;

    VkPipelineLayout GetLayout() const
    {
        return layout;
    }

private:
    VkPipelineCreateFlags           createFlags;
    VkPipelineShaderStageCreateInfo shaderStage;
    VkPipelineLayout                layout;
};

class RayTracingPipelineBuilder
{
    // Empty
};
} // namespace sy::vk
