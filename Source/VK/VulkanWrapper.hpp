#pragma once
#define VK_DESTROY_LAMBDA_SIGNATURE(HANDLE_TYPE) [](const VulkanRHI& vulkanRHI, HANDLE_TYPE handle)

namespace sy::vk
{
	class VulkanRHI;

	template <typename VulkanHandleType>
	class VulkanWrapper : public NamedType, public NonCopyable
	{
	public:
		using VulkanDestroyFunction_t = std::function<void(const VulkanRHI& VulkanRHI, VulkanHandleType)>;
		using Native_t = VulkanHandleType;

	public:
		VulkanWrapper(const std::string_view name, const VulkanRHI& vulkanRHI, const VkObjectType type) :
			VulkanWrapper(name, vulkanRHI, type, VK_DESTROY_LAMBDA_SIGNATURE(Native_t)
			{
			})
		{
		}

		VulkanWrapper(const std::string_view name, const VulkanRHI& vulkanRHI, const VkObjectType type,
		              const VulkanDestroyFunction_t destroyFunction) :
			NamedType(name),
			vulkanRHI(vulkanRHI),
			type(type),
			destroyFunction(destroyFunction)
		{
		}

		virtual ~VulkanWrapper()
		{
			destroyFunction(vulkanRHI, handle);
		}

		[[nodiscard]] Native_t GetNativeHandle() const
		{
			return handle;
		}

		[[nodiscard]] VkObjectType GetType() const
		{
			return type;
		}

		[[nodiscard]] const VulkanRHI& GetRHI() const
		{
			return vulkanRHI;
		}

	protected:
		void UpdateHandle(const Native_t newHandle)
		{
			SY_ASSERT(newHandle != VK_NULL_HANDLE, "Invalid new vulkan handle.");
			SY_ASSERT(handle == VK_NULL_HANDLE, "Vulkan Handle Overwrite!");
			handle = newHandle;
		}

	private:
		const VulkanRHI& vulkanRHI;
		const VulkanDestroyFunction_t destroyFunction;
		const VkObjectType type;
		Native_t handle = VK_NULL_HANDLE;
	};

	template <typename VulkanWrapperType>
	std::vector<typename VulkanWrapperType::Native_t> TransformVulkanWrappersToNativesWithValidation(
		const CRefSpan<VulkanWrapperType> wrappers,
		const std::function<bool(const CRef<VulkanWrapperType> wrapper)> validation)
	{
		std::vector<typename VulkanWrapperType::Native_t> natives;
		natives.resize(wrappers.size());
		std::transform(wrappers.begin(), wrappers.end(),
		               natives.begin(),
		               [&validation](const CRef<VulkanWrapperType> wrapper) -> VulkanWrapperType::Native_t
		               {
			               const bool bIsValid = validation(wrapper);
			               SY_ASSERT(bIsValid, "Invalid wrapper transformation.");
			               return wrapper.get().GetNativeHandle();
		               });

		return natives;
	}

	template <typename VulkanWrapperType>
	std::vector<typename VulkanWrapperType::Native_t> TransformVulkanWrappersToNatives(
		const CRefSpan<VulkanWrapperType> wrappers)
	{
		return TransformVulkanWrappersToNativesWithValidation<VulkanWrapperType>(wrappers,
			[](const CRef<VulkanWrapperType>)
			{
				return true;
			});
	}
}
