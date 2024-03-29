#include <PCH.h>
#include <VK/ShaderModule.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>

namespace sy
{
namespace vk
{
ShaderModule::ShaderModule(const std::string_view name, VulkanContext& vulkanContext, const std::string_view filePath, const VkShaderStageFlagBits shaderType, const std::string_view entryPoint) :
    VulkanWrapper<VkShaderModule>(name, vulkanContext, VK_OBJECT_TYPE_SEMAPHORE), path(filePath), entryPoint(entryPoint), shaderType(shaderType)
{
    SY_ASSERT(!filePath.empty(), "Empty shader file path!");
    SY_ASSERT(!entryPoint.empty(), "Empty shader entry point!");

    std::ifstream file(path, std::ios::ate | std::ios::binary);
    SY_ASSERT(file.is_open(), "Failed to open shader binary from {}.", path);

    const size_t          fileSize = file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    const VkShaderModuleCreateInfo createInfo{
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .codeSize = buffer.size() * sizeof(uint32_t),
        .pCode    = buffer.data()};

    spdlog::trace("Creating shader module from {}...", path);

    NativeHandle handle    = VK_NULL_HANDLE;
    const auto&  vulkanRHI = vulkanContext.GetRHI();
    VK_ASSERT(vkCreateShaderModule(vulkanRHI.GetDevice(), &createInfo, nullptr, &handle), "Failed to create shader module from {}.", path);

    UpdateHandle(
        handle,
        [handle](const VulkanRHI& rhi) {
            vkDestroyShaderModule(rhi.GetDevice(), handle, nullptr);
        });
}
} // namespace vk
} // namespace sy
