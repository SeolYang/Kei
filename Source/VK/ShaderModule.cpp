#include <Core/Core.h>
#include <VK/ShaderModule.h>
#include <VK/VulkanContext.h>

namespace sy
{
	namespace vk
	{
		ShaderModule::ShaderModule(const std::string_view name, const VulkanContext& vulkanContext, const std::string_view filePath, const VkShaderStageFlagBits shaderType, const std::string_view entryPoint) :
			VulkanWrapper<VkShaderModule>(name, vulkanContext, VK_OBJECT_TYPE_SEMAPHORE, VK_DESTROY_LAMBDA_SIGNATURE(VkShaderModule)
		{
			vkDestroyShaderModule(vulkanContext.GetDevice(), handle, nullptr);
		}),
			path(filePath),
			entryPoint(entryPoint),
			shaderType(shaderType)
		{
			SY_ASSERT(!filePath.empty(), "Empty shader file path!");
			SY_ASSERT(!entryPoint.empty(), "Empty shader entry point!");

			std::ifstream file(path, std::ios::ate | std::ios::binary);
			SY_ASSERT(file.is_open(), "Failed to open shader binary from {}.", path);

			const size_t fileSize = file.tellg();
			std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

			file.seekg(0);
			file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
			file.close();

			const VkShaderModuleCreateInfo createInfo
			{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.pNext = nullptr,
				.codeSize = buffer.size() * sizeof(uint32_t),
				.pCode = buffer.data()
			};

			spdlog::trace("Creating shader module from {}...", path);
			VK_ASSERT(vkCreateShaderModule(vulkanContext.GetDevice(), &createInfo, nullptr, &handle), "Failed to create shader module from {}.", path);
		}
	}
}
