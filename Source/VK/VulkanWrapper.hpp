#pragma once

namespace sy
{
	class VulkanInstance;

	template <typename VulkanHandleType>
	class VulkanWrapper : public NamedType
	{
		using VulkanDestroyFunctionType = std::function<void(const VulkanInstance& vulkanInstance, VulkanHandleType)>;

	public:
		VulkanWrapper(const std::string_view name, const VulkanInstance& vulkanInstance, const VulkanDestroyFunctionType destroyFunction) :
			NamedType(name),
			vulkanInstance(vulkanInstance),
			destroyFunction(destroyFunction)
		{
		}

		virtual ~VulkanWrapper()
		{
			destroyFunction(vulkanInstance, handle);
		}

		[[nodiscard]] VulkanHandleType GetNativeHandle() const { return handle; }

	protected:
		const VulkanInstance& vulkanInstance;
		const VulkanDestroyFunctionType destroyFunction;
		VulkanHandleType handle = VK_NULL_HANDLE;

	};
}

#define VK_DESTROY_LAMBDA_SIGNATURE(HANDLE_TYPE) [](const VulkanInstance& vulkanInstance, HANDLE_TYPE handle)