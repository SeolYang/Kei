#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>

namespace sy::vk
{
	class ShaderModule : public VulkanWrapper<VkShaderModule>
	{
	public:
		ShaderModule(std::string_view name, VulkanContext& vulkanContext, std::string_view filePath,
			VkShaderStageFlagBits shaderType, std::string_view entryPoint);
		~ShaderModule() override = default;

		[[nodiscard]] auto GetShaderType() const
		{
			return shaderType;
		}

		[[nodiscard]] std::string_view GetPath() const
		{
			return path;
		}

		[[nodiscard]] std::string_view GetEntryPoint() const
		{
			return entryPoint;
		}

	private:
		std::string path;
		std::string entryPoint;
		const VkShaderStageFlagBits shaderType;
	};
} // namespace sy::vk
