#pragma once
#include <PCH.h>
#include <VK/VulkanContext.h>

namespace sy::vk
{
	class VulkanRHI;
	class VulkanContext;

	/**
	 *	Usage: [=](){ vkDestroy...(handle); ...vmaDestroy...(allocation); }
	 *  Capture native handle and allocation as value. It design to enqueue	deleter to vulkan context deallocation queue when wrapper object destructed.
	 *  Then, it'll be deallocate vulkan object(or vulkan memory allocator allocation) as automatically at next begin frame.
	 */
	template <typename VulkanHandleType>
	class VulkanWrapper : public NamedType, public NonCopyable
	{
	public:
		using NativeHandle = VulkanHandleType;

	public:
		virtual ~VulkanWrapper()
		{
			SY_ASSERT(deleter, "Deleter for vulkan wrapper must be specified.");
			/** @todo enqueue to deferred deallocation queue of vulkan context. */
			if (deleter)
			{
				vulkanContext.EnqueueDeferredDeallocation(std::move(*deleter));
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

		[[nodiscard]] VulkanContext& GetContext() const
		{
			return vulkanContext;
		}

		[[nodiscard]] const VulkanRHI& GetRHI() const
		{
			return vulkanContext.GetRHI();
		}

	protected:
		VulkanWrapper(const std::string_view name, VulkanContext& vulkanContext, const VkObjectType type)
			: NamedType(name), vulkanContext(vulkanContext), type(type)
		{
		}

		void UpdateHandle(const NativeHandle newHandle, const std::optional<VulkanObjectDeleter> deleter)
		{
			SY_ASSERT(newHandle != VK_NULL_HANDLE, "Invalid new vulkan handle.");
			SY_ASSERT(handle == VK_NULL_HANDLE, "Vulkan Handle Overwrite!");
			this->handle = newHandle;
			this->deleter = deleter;
		}

	private:
		VulkanContext& vulkanContext;
		const VkObjectType type;
		NativeHandle handle = VK_NULL_HANDLE;
		std::optional<VulkanObjectDeleter> deleter = std::nullopt;
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

#define SY_VK_WRAPPER_EMPTY_DELETER [=](const VulkanRHI&) {}
} // namespace sy::vk
