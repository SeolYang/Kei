#include <PCH.h>
#include <VK/VertexInputBuilder.h>

namespace sy::vk
{
VertexInputBuilder& VertexInputBuilder::Clear()
{
    vertexInputAttributeDescriptions.clear();
    vertexInputBindingDescriptions.clear();
    return *this;
}

VertexInputBuilder& VertexInputBuilder::AddVertexInputAttribute(const uint32_t location, const uint32_t binding, const VkFormat format, const uint32_t offset)
{
    vertexInputAttributeDescriptions.emplace_back(location, binding, format, offset);
    return *this;
}

VkPipelineVertexInputStateCreateInfo VertexInputBuilder::Build() const
{
    return {
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                           = nullptr,
        .flags                           = 0,
        .vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexInputBindingDescriptions.size()),
        .pVertexBindingDescriptions      = vertexInputBindingDescriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size()),
        .pVertexAttributeDescriptions    = vertexInputAttributeDescriptions.data()};
}
} // namespace sy::vk
