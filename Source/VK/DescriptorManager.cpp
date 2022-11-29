#include <Core.h>
#include <VK/DescriptorManager.h>
#include <VK/VulkanInstance.h>

namespace sy
{
	DescriptorManager::DescriptorManager(const VulkanInstance& vulkanInstance) :
		vulkanInstance(vulkanInstance)
	{
		DescriptorPoolSizeBuilder poolSizeBuilder;
		poolSizeBuilder.AddDescriptors(EDescriptorType::CombinedImageSampler, 1000)
			.AddDescriptors(EDescriptorType::UniformBuffer, 1000)
			.AddDescriptors(EDescriptorType::StorageBuffer, 1000);

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
		ranges::fill(bindingFlags.begin(), bindingFlags.end(),
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
				return poolSize.Size;
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
		VK_ASSERT(vkCreateDescriptorSetLayout(vulkanInstance.GetLogicalDevice(), &bindlessLayoutInfo, nullptr, &bindlessLayout), "Failed to create bindless descriptor set layout.");

		RefVec<PoolPackage> poolPackageRefs;
		poolPackageRefs.emplace_back(persistentDescriptorPackage);
		for (auto& transientDescriptorPackage : transientDescriptorPackages)
		{
			poolPackageRefs.emplace_back(transientDescriptorPackage);
		}

		spdlog::trace("Creating pool packages...");
		for (auto& poolPackageRef : poolPackageRefs)
		{
			auto& poolPackage = poolPackageRef.get();
			VK_ASSERT(vkCreateDescriptorPool(vulkanInstance.GetLogicalDevice(), &poolCreateInfo, nullptr, &poolPackage.DescriptorPool), "Failed to create descriptor pool.");

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
				.descriptorPool = poolPackage.DescriptorPool,
				.descriptorSetCount = 1,
				.pSetLayouts = &bindlessLayout
			};

			VK_ASSERT(vkAllocateDescriptorSets(vulkanInstance.GetLogicalDevice(), &setAllocateInfo, &poolPackage.DescriptorSet), "Failed to allocate descriptor set.");
		}
	}

	DescriptorManager::~DescriptorManager()
	{
		for (const auto& poolPackage : transientDescriptorPackages)
		{
			vkDestroyDescriptorPool(vulkanInstance.GetLogicalDevice(), poolPackage.DescriptorPool, nullptr);
		}

		vkDestroyDescriptorSetLayout(vulkanInstance.GetLogicalDevice(), bindlessLayout, nullptr);
	}

	void DescriptorManager::BeginFrame(const size_t currentInFlightFrameIdx)
	{
	}

	void DescriptorManager::EndFrame(const size_t currentInFlightFrameIdx)
	{
	}
}
