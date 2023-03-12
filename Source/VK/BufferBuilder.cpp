#include <PCH.h>
#include <VK/BufferBuilder.h>
#include <VK/Buffer.h>

namespace sy::vk
{
	bool BufferBuilder::IsValidToBuild() const
	{
		const bool bIsValidSize = size > 0;
		const bool bIsValidUsage = usage.has_value();
		const bool bIsValidMemoryUsage = memoryUsage.has_value();
		SY_ASSERT(bIsValidSize, "BufferBuilder has invalid size to build buffer.");
		SY_ASSERT(bIsValidUsage, "BufferBuilder has invalid usage to build buffer.");
		SY_ASSERT(bIsValidMemoryUsage, "BufferBuilder has invalid memory usage to build buffer.");

		return bIsValidSize && bIsValidUsage && bIsValidMemoryUsage;
	}

	std::unique_ptr<Buffer> BufferBuilder::Build() const
	{
		SY_ASSERT(IsValidToBuild(), "Buffer builder is not valid to build.");
		return std::make_unique<Buffer>(*this);
	}

	BufferBuilder BufferBuilder::UniformBufferTemplate(const VulkanContext& vulkanContext)
	{
		return BufferBuilder{ vulkanContext }
			.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
			.SetMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
			.SetTargetInitialState(EBufferState::General);
	}

	BufferBuilder BufferBuilder::StorageBufferTemplate(const VulkanContext& vulkanContext)
	{
		return BufferBuilder{ vulkanContext }
			.SetUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
			.SetMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
			.SetTargetInitialState(EBufferState::General);
	}

	BufferBuilder BufferBuilder::StagingBufferTemplate(const VulkanContext& vulkanContext)
	{
		return BufferBuilder{ vulkanContext }
			.SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			.SetMemoryUsage(VMA_MEMORY_USAGE_CPU_ONLY);
	}

	BufferBuilder BufferBuilder::VertexBufferTemplate(const VulkanContext& vulkanContext)
	{
		return BufferBuilder{ vulkanContext }
			.SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
			.SetMemoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetTargetInitialState(EBufferState::VertexBuffer);
	}

	BufferBuilder BufferBuilder::IndexBufferTemplate(const VulkanContext& vulkanContext)
	{
		return BufferBuilder{ vulkanContext }
			.SetUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
			.SetMemoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetTargetInitialState(EBufferState::IndexBuffer);
	}
}
