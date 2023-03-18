#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanRHI;

	template <typename VulkanHandleType>
	class VulkanWrapper : public NamedType, public NonCopyable
	{
	public:
		using Deleter = std::function<void(const VulkanRHI&)>;
		using NativeHandle = VulkanHandleType;

	public:
		virtual ~VulkanWrapper()
		{
			SY_ASSERT(deleter, "Deleter for vulkan wrapper must be specified.");
			/** @todo enqueue to deferred deallocation queue of vulkan context. */
			if (deleter)
			{
				(*deleter)(vulkanRHI);
			}
		}

		[[nodiscard]] NativeHandle GetNative() const
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
		VulkanWrapper(const std::string_view name, const VulkanRHI& vulkanRHI, const VkObjectType type)
			: NamedType(name), vulkanRHI(vulkanRHI), type(type)
		{
		}

		void UpdateHandle(const NativeHandle newHandle, const std::optional<Deleter> deleter)
		{
			SY_ASSERT(newHandle != VK_NULL_HANDLE, "Invalid new vulkan handle.");
			SY_ASSERT(handle == VK_NULL_HANDLE, "Vulkan Handle Overwrite!");
			this->handle = newHandle;
			this->deleter = deleter;
		}

	private:
		const VulkanRHI& vulkanRHI;
		const VkObjectType type;
		NativeHandle handle = VK_NULL_HANDLE;
		std::optional<Deleter> deleter = std::nullopt;
	};

	template <typename VulkanWrapperType>
	std::vector<typename VulkanWrapperType::NativeHandle> TransformVulkanWrappersToNativesWithValidation(
		const CRefSpan<VulkanWrapperType> wrappers,
		const std::function<bool(const CRef<VulkanWrapperType> wrapper)> validation)
	{
		std::vector<typename VulkanWrapperType::NativeHandle> natives;
		natives.resize(wrappers.size());
		std::transform(wrappers.begin(), wrappers.end(),
			natives.begin(),
			[&validation](const CRef<VulkanWrapperType> wrapper) -> VulkanWrapperType::NativeHandle {
				const bool bIsValid = validation(wrapper);
				SY_ASSERT(bIsValid, "Invalid wrapper transformation.");
				return wrapper.get().GetNative();
			});

		return natives;
	}

	template <typename VulkanWrapperType>
	std::vector<typename VulkanWrapperType::NativeHandle> TransformVulkanWrappersToNatives(
		const CRefSpan<VulkanWrapperType> wrappers)
	{
		return TransformVulkanWrappersToNativesWithValidation<VulkanWrapperType>(wrappers,
			[](const CRef<VulkanWrapperType>) {
				return true;
			});
	}

	/**
	 *	Usage: [=](){ vkDestroy...(handle); ...vmaDestroy...(allocation); }
	 *  Capture native handle and allocation as value. It design to enqueue	deleter to vulkan context deallocation queue when wrapper object destructed.
	 *  Then, it'll be deallocate vulkan object(or vulkan memory allocator allocation) as automatically at next begin frame.
	 */
#define SY_VK_WRAPPER_DELETER(RHI_PARAMETER_NAME) [=](const VulkanRHI&##RHI_PARAMETER_NAME)
#define SY_VK_WRAPPER_EMPTY_DELETER [=](const VulkanRHI&) {}
} // namespace sy::vk
