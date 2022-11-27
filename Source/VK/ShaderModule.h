#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class ShaderModule : public VulkanWrapper<VkShaderModule>
	{
	public:
		ShaderModule(std::string_view name, const VulkanInstance& vulkanInstance, std::string_view filePath, VkShaderStageFlagBits shaderType, std::string_view entryPoint);
		virtual ~ShaderModule() override = default;

		[[nodiscard]] auto GetShaderType() const { return shaderType; }
		[[nodiscard]] std::string_view GetPath() const { return path; }
		[[nodiscard]] std::string_view GetEntryPoint() const { return entryPoint; }

	private:
		std::string path;
		std::string entryPoint;
		const VkShaderStageFlagBits shaderType;

	};
}