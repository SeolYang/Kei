#pragma once

namespace sy
{
	class VulkanInstance;

	template <typename VulkanHandleType>
	class VulkanWrapper : public NamedType
	{
	public:
		using VulkanDestroyFunction_t = std::function<void(const VulkanInstance& vulkanInstance, VulkanHandleType)>;
		using Native_t = VulkanHandleType;

	public:
		VulkanWrapper(const std::string_view name, const VulkanInstance& vulkanInstance, const VulkanDestroyFunction_t destroyFunction) :
			NamedType(name),
			vulkanInstance(vulkanInstance),
			destroyFunction(destroyFunction)
		{
		}

		virtual ~VulkanWrapper()
		{
			destroyFunction(vulkanInstance, handle);
		}

		[[nodiscard]] Native_t GetNativeHandle() const { return handle; }

	protected:
		const VulkanInstance& vulkanInstance;
		const VulkanDestroyFunction_t destroyFunction;
		Native_t handle = VK_NULL_HANDLE;

	};

	template <typename VulkanWrapperType>
	std::vector<typename VulkanWrapperType::Native_t> TransformVulkanWrappersToNatives(const CRefSpan<VulkanWrapperType> wrappers)
	{
		std::vector<typename VulkanWrapperType::Native_t> natives;
		natives.resize(wrappers.size());
		std::transform(wrappers.begin(), wrappers.end(),
			natives.begin(),
			[](const CRef<VulkanWrapperType> wrapper) -> VulkanWrapperType::Native_t
			{
				return wrapper.get().GetNativeHandle();
			});

		return natives;
	}
}

#define VK_DESTROY_LAMBDA_SIGNATURE(HANDLE_TYPE) [](const VulkanInstance& vulkanInstance, HANDLE_TYPE handle)