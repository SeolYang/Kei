#include <Core.h>
#include <VK/DescriptorPool.h>
#include <VK/VulkanInstance.h>

namespace sy
{
	DescriptorPool::DescriptorPool(const std::string_view name, const VulkanInstance& vulkanInstance) :
		VulkanWrapper(name, vulkanInstance, VK_DESTROY_LAMBDA_SIGNATURE(VkDescriptorPool)
		{
			vkDestroyDescriptorPool(vulkanInstance.GetLogicalDevice(), handle, nullptr);
		}),
		uniformBufferPool(1, 16)
	{
		constexpr VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, MaxBindlessResourcesPerDescriptor },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MaxBindlessResourcesPerDescriptor },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MaxBindlessResourcesPerDescriptor },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MaxBindlessResourcesPerDescriptor },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MaxBindlessResourcesPerDescriptor },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MaxBindlessResourcesPerDescriptor },
		};

		const VkDescriptorPoolCreateInfo createInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = MaxBindlessResourcesPerDescriptor * static_cast<uint32_t>(LengthOfArray(poolSizes)),
			.poolSizeCount = static_cast<uint32_t>(LengthOfArray(poolSizes)),
			.pPoolSizes = poolSizes
		};

		spdlog::trace("Creating bindless descriptor pool...");
		VK_ASSERT(vkCreateDescriptorPool(vulkanInstance.GetLogicalDevice(), &createInfo, nullptr, &handle), "Failed to create descriptor pool.");

		VkDescriptorBindingFlags bindlessFlag = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
		std::array<VkDescriptorBindingFlags, LengthOfArray(poolSizes)> bindlessFlags;
		bindlessFlags.fill(bindlessFlag);
		bindlessFlags.back() |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;

		std::array<VkDescriptorSetLayoutBinding, LengthOfArray(poolSizes)> bindings;
		for (size_t idx = 0; idx < LengthOfArray(poolSizes); ++idx)
		{
			bindings[idx] =
			{
				.binding = QueryBindlessBindingOfDescriptor(poolSizes[idx].type),
				.descriptorType = poolSizes[idx].type,
				.descriptorCount = poolSizes[idx].descriptorCount,
				.stageFlags = VK_SHADER_STAGE_ALL,
				.pImmutableSamplers = nullptr,
			};
		}

		const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
			.bindingCount = bindlessFlags.size(),
			.pBindingFlags = bindlessFlags.data()
		};

		const VkDescriptorSetLayoutCreateInfo layoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &extendedInfo,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
			.bindingCount = bindings.size(),
			.pBindings = bindings.data()
		};

		spdlog::trace("Creating bindless descriptor set layout...");
		VK_ASSERT(vkCreateDescriptorSetLayout(vulkanInstance.GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout), "Failed to create descriptor set layout.");

		const uint32_t maxBinding = MaxBindlessResourcesPerDescriptor - 1;
		const VkDescriptorSetVariableDescriptorCountAllocateInfoEXT descriptorSetVariableDescriptorCountAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
			.pNext = nullptr,
			.descriptorSetCount = 1,
			.pDescriptorCounts = &maxBinding
		};

		const VkDescriptorSetAllocateInfo setAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = &descriptorSetVariableDescriptorCountAllocateInfo,
			.descriptorPool = handle,
			.descriptorSetCount = 1,
			.pSetLayouts = &descriptorSetLayout
		};

		spdlog::trace("Creating bindless descriptor set...");
		VK_ASSERT(vkAllocateDescriptorSets(vulkanInstance.GetLogicalDevice(), &setAllocateInfo, &descriptorSet), "Failed to allocate descriptor set.");
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorSetLayout(vulkanInstance.GetLogicalDevice(), descriptorSetLayout, nullptr);
	}

	UniqueHandle<OffsetPool::Slot_t> DescriptorPool::RequestUniformBufferDescriptorIndex()
	{
		const auto allocated = uniformBufferPool.Allocate();
		SY_ASSERT(allocated.Offset > MaxBindlessResourcesPerDescriptor, "Uniform buffer desciprotr allocation num reach to maximum.");
		return {
			allocated,
			[this](OffsetPool::Slot_t slot)
			{
				pendingUniformBufferDeallocations.emplace_back(slot);
			}
		};
	}

	void DescriptorPool::EndFrame()
	{
		for (auto& dealloc : pendingUniformBufferDeallocations)
		{
			uniformBufferPool.Deallocate(dealloc);
		}
		pendingUniformBufferDeallocations.clear();
	}
}
