#include <Core.h>
#include <DescriptorManager.h>
#include <FrameTracker.h>
#include <VK/VulkanContext.h>
#include <VK/Buffer.h>
#include <VK/Texture.h>

namespace sy
{
	DescriptorManager::DescriptorManager(const VulkanContext& vulkanContext, const FrameTracker& frameTracker) :
		vulkanContext(vulkanContext),
		frameTracker(frameTracker)
	{
		DescriptorPoolSizeBuilder poolSizeBuilder;
		poolSizeBuilder.AddDescriptors(EDescriptorType::CombinedImageSampler, 1000)
			.AddDescriptors(EDescriptorType::UniformBuffer, 1000)
			.AddDescriptors(EDescriptorType::StorageBuffer, 1000)
			.AddDescriptors(EDescriptorType::SampledImage, 1000)
			.AddDescriptors(EDescriptorType::CombinedImageSampler, 1000)
			.AddDescriptors(EDescriptorType::StorageImage, 1000);

		const auto nativePoolSizes = poolSizeBuilder.BuildAsNative();
		const auto poolSizes = poolSizeBuilder.Build();

		const VkDescriptorPoolCreateInfo poolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = static_cast<uint32_t>(poolSizeBuilder.GetDescriptorCount()),
			.poolSizeCount = static_cast<uint32_t>(nativePoolSizes.size()),
			.pPoolSizes = nativePoolSizes.data()
		};

		std::vector<VkDescriptorBindingFlags> bindingFlags;
		bindingFlags.resize(nativePoolSizes.size());
		std::fill(bindingFlags.begin(), bindingFlags.end(),
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT);
		bindingFlags.back() |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		bindings.resize(nativePoolSizes.size());
		std::vector<uint32_t> descriptorCounts;
		descriptorCounts.resize(nativePoolSizes.size());
		std::transform(poolSizes.begin(), poolSizes.end(),
			bindings.begin(),
			[](const DescriptorPoolSize& poolSize)
			{
				return
					VkDescriptorSetLayoutBinding
				{
					ToUnderlying(poolSize.Type),
					ToNative(poolSize.Type),
					static_cast<uint32_t>(poolSize.Size),
					VK_SHADER_STAGE_ALL,
					nullptr
				};
			});

		std::transform(poolSizes.begin(), poolSizes.end(),
			descriptorCounts.begin(),
			[](const DescriptorPoolSize& poolSize)
			{
				return static_cast<uint32_t>(poolSize.Size);
			});

		const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedLayoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
			.bindingCount = static_cast<uint32_t>(bindingFlags.size()),
			.pBindingFlags = bindingFlags.data()
		};

		const VkDescriptorSetLayoutCreateInfo bindlessLayoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &extendedLayoutInfo,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings = bindings.data()
		};

		spdlog::trace("Creating Bindless descriptor set layout...");
		VK_ASSERT(vkCreateDescriptorSetLayout(vulkanContext.GetDevice(), &bindlessLayoutInfo, nullptr, &bindlessLayout), "Failed to create bindless descriptor set layout.");


		spdlog::trace("Creating pool package...");
		VK_ASSERT(vkCreateDescriptorPool(vulkanContext.GetDevice(), &poolCreateInfo, nullptr, &descriptorPoolPackage.DescriptorPool), "Failed to create descriptor pool.");

		const VkDescriptorSetVariableDescriptorCountAllocateInfoEXT descriptorSetVariableDescriptorCountAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
			.pNext = nullptr,
			.descriptorSetCount = 1,
			.pDescriptorCounts = descriptorCounts.data()
		};

		const VkDescriptorSetAllocateInfo setAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = &descriptorSetVariableDescriptorCountAllocateInfo,
			.descriptorPool = descriptorPoolPackage.DescriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &bindlessLayout
		};

		VK_ASSERT(vkAllocateDescriptorSets(vulkanContext.GetDevice(), &setAllocateInfo, &descriptorPoolPackage.DescriptorSet), "Failed to allocate descriptor set.");

		for (const auto& poolSize : poolSizes)
		{
			auto& offsetPoolPackage = descriptorPoolPackage.OffsetPoolPackages[ToUnderlying(poolSize.Type)];
			offsetPoolPackage.Pool.Grow(poolSize.Size);
			offsetPoolPackage.AllocatedSlots.resize(poolSize.Size);
		}
	}

	DescriptorManager::~DescriptorManager()
	{
		vkDestroyDescriptorPool(vulkanContext.GetDevice(), descriptorPoolPackage.DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(vulkanContext.GetDevice(), bindlessLayout, nullptr);
	}

	void DescriptorManager::BeginFrame()
	{
		auto& pendingList = pendingDeallocations[frameTracker.GetCurrentInFlightFrameIndex()];
		for (const Allocation& allocation : pendingList)
		{
			allocation.Owner.Pool.Deallocate(allocation.AllocatedSlot);
		}
		pendingList.clear();
	}

	void DescriptorManager::EndFrame()
	{
		combinedWriteDescriptorSets.reserve(bufferWriteDescriptors.size() + imageWriteDescriptors.size());
		if (!bufferWriteDescriptors.empty())
		{
			for (size_t idx = 0; idx < bufferWriteDescriptors.size(); ++idx)
			{
				bufferWriteDescriptors[idx].pBufferInfo = &bufferInfos[idx];
			}

			combinedWriteDescriptorSets.assign(bufferWriteDescriptors.begin(), bufferWriteDescriptors.end());
		}

		if (!imageWriteDescriptors.empty())
		{
			for (size_t idx = 0; idx < imageWriteDescriptors.size(); ++idx)
			{
				imageWriteDescriptors[idx].pImageInfo = &imageInfos[idx];
			}

			combinedWriteDescriptorSets.insert(combinedWriteDescriptorSets.end(), imageWriteDescriptors.begin(), imageWriteDescriptors.end());
		}

		if (!combinedWriteDescriptorSets.empty())
		{
			vkUpdateDescriptorSets(vulkanContext.GetDevice(), static_cast<uint32_t>(combinedWriteDescriptorSets.size()), combinedWriteDescriptorSets.data(), 0, nullptr);
			bufferWriteDescriptors.clear();
			bufferInfos.clear();
			imageWriteDescriptors.clear();
			imageInfos.clear();
			combinedWriteDescriptorSets.clear();
		}
	}

	OffsetSlotPtr DescriptorManager::RequestDescriptor(const Buffer& buffer, const bool bIsDynamic)
	{
		const auto descriptorType = BufferUsageToDescriptorType(buffer.GetUsage(), bIsDynamic);
		const auto descriptorBinding = ToUnderlying(descriptorType);
		auto& offsetPoolPackage = descriptorPoolPackage.OffsetPoolPackages[descriptorBinding];

		auto& allocatedSlots = offsetPoolPackage.AllocatedSlots;
		size_t slotOffset;
		{
			std::lock_guard lock{ offsetPoolPackage.Mutex };
			const FixedOffsetPool::Slot_t allocatedSlot = offsetPoolPackage.Pool.Allocate();
			slotOffset = allocatedSlot.Offset;
			allocatedSlots[slotOffset] = allocatedSlot;
		}

		// Add new write descriptor set to write descriptor set list 
		{
			std::lock_guard lock{ bufferWriteDescriptorMutex };

			const VkWriteDescriptorSet writeDescriptorSet
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = GetDescriptorSet(),
				.dstBinding = descriptorBinding,
				.dstArrayElement = static_cast<uint32_t>(slotOffset),
				.descriptorCount = 1,
				.descriptorType = ToNative(descriptorType),
				.pImageInfo = nullptr,
				.pTexelBufferView = nullptr
			};

			bufferInfos.emplace_back(buffer.GetDescriptorInfo());
			bufferWriteDescriptors.emplace_back(writeDescriptorSet);
		}

		return{
				&allocatedSlots[slotOffset],
				[this, &offsetPoolPackage](const FixedOffsetPool::Slot_t* slotPtr)
				{
					auto& pendingList = pendingDeallocations[frameTracker.GetCurrentInFlightFrameIndex()];
					auto& pendingListMutex = pendingMutexList[frameTracker.GetCurrentInFlightFrameIndex()];
					std::lock_guard lock{ pendingListMutex };
					pendingList.emplace_back(*slotPtr, offsetPoolPackage);
				}
		};
	}

	OffsetSlotPtr DescriptorManager::RequestDescriptor(const Texture& texture, const bool bIsCombinedSampler)
	{
		const auto descriptorType = ImageUsageToDescriptorType(texture.GetUsage(), bIsCombinedSampler);
		const auto descriptorBinding = ToUnderlying(descriptorType);
		auto& offsetPoolPackage = descriptorPoolPackage.OffsetPoolPackages[descriptorBinding];

		auto& allocatedSlots = offsetPoolPackage.AllocatedSlots;
		size_t slotOffset;
		{
			std::lock_guard lock{ offsetPoolPackage.Mutex };
			const FixedOffsetPool::Slot_t allocatedSlot = offsetPoolPackage.Pool.Allocate();
			slotOffset = allocatedSlot.Offset;
			allocatedSlots[slotOffset] = allocatedSlot;
		}

		// Add new write descriptor set to write descriptor set list 
		{
			std::lock_guard lock{ imageWriteDescriptorMutex };

			const VkWriteDescriptorSet writeDescriptorSet
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = GetDescriptorSet(),
				.dstBinding = descriptorBinding,
				.dstArrayElement = static_cast<uint32_t>(slotOffset),
				.descriptorCount = 1,
				.descriptorType = ToNative(descriptorType),
				.pImageInfo = nullptr,
				.pTexelBufferView = nullptr
			};

			imageInfos.emplace_back(texture.GetDescriptorInfo());
			imageWriteDescriptors.emplace_back(writeDescriptorSet);
		}

		return{
			&allocatedSlots[slotOffset],
			[this, &offsetPoolPackage](const FixedOffsetPool::Slot_t* slotPtr)
			{
				auto& pendingList = pendingDeallocations[frameTracker.GetCurrentInFlightFrameIndex()];
				auto& pendingListMutex = pendingMutexList[frameTracker.GetCurrentInFlightFrameIndex()];
				std::lock_guard lock{ pendingListMutex };
				pendingList.emplace_back(*slotPtr, offsetPoolPackage);
			}
		};
	}
}
