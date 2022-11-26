#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class ShaderModule : public VulkanWrapper<VkShaderModule>
	{
	public:
		ShaderModule(std::string_view name, const VulkanInstance& vulkanInstance, std::string_view filePath, VkShaderStageFlags shaderType, std::string_view entryPoint);
		virtual ~ShaderModule() override = default;

		[[nodiscard]] auto GetShaderType() const { return shaderType; }
		[[nodiscard]] auto GetPath() const { return path; }

	private:
		std::string path;
		std::string entryPoint;
		const VkShaderStageFlags shaderType;

	};
}