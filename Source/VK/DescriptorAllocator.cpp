#include <PCH.h>
#include <VK/DescriptorAllocator.h>
#include <VK/VulkanContext.h>
#include <VK/FrameTracker.h>
#include <VK/VulkanRHI.h>
#include <VK/Buffer.h>
#include <VK/Texture.h>
#include <VK/TextureView.h>
#include <VK/Sampler.h>

namespace sy
{
namespace vk
{
DescriptorAllocator::DescriptorAllocator(VulkanContext& vulkanContext, const FrameTracker& frameTracker) :
    vulkanContext(vulkanContext), frameTracker(frameTracker)
{
}

DescriptorAllocator::~DescriptorAllocator()
{
}

void DescriptorAllocator::Startup()
{
    spdlog::info("Startup Descriptor Manager.");
    const auto poolSizeBuilder = DescriptorPoolSizeBuilder{}
                                     .AddDescriptors(vk::EDescriptorType::CombinedImageSampler, 1000)
                                     .AddDescriptors(vk::EDescriptorType::UniformBuffer, 1000)
                                     .AddDescriptors(vk::EDescriptorType::StorageBuffer, 1000)
                                     .AddDescriptors(vk::EDescriptorType::SampledImage, 1000)
                                     .AddDescriptors(vk::EDescriptorType::CombinedImageSampler, 1000)
                                     .AddDescriptors(vk::EDescriptorType::StorageImage, 1000);

    const auto nativePoolSizes = poolSizeBuilder.BuildAsNative();
    const auto poolSizes       = poolSizeBuilder.Build();

    const VkDescriptorPoolCreateInfo poolCreateInfo{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets       = static_cast<uint32_t>(poolSizeBuilder.GetDescriptorCount()),
        .poolSizeCount = static_cast<uint32_t>(nativePoolSizes.size()),
        .pPoolSizes    = nativePoolSizes.data()};

    std::vector<VkDescriptorBindingFlags> bindingFlags;
    bindingFlags.resize(nativePoolSizes.size());

    constexpr auto flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    std::fill(bindingFlags.begin(), bindingFlags.end(), flags);
    bindingFlags.back() = (flags | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT);

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.resize(nativePoolSizes.size());
    std::vector<uint32_t> descriptorCounts;
    descriptorCounts.resize(nativePoolSizes.size());
    std::transform(poolSizes.begin(), poolSizes.end(),
                   bindings.begin(),
                   [](const DescriptorPoolSize& poolSize) {
                       return VkDescriptorSetLayoutBinding{
                           ToUnderlying(poolSize.Type),
                           ToNative(poolSize.Type),
                           static_cast<uint32_t>(poolSize.Size),
                           VK_SHADER_STAGE_ALL,
                           nullptr};
                   });

    std::transform(poolSizes.begin(), poolSizes.end(),
                   descriptorCounts.begin(),
                   [](const DescriptorPoolSize& poolSize) {
                       return static_cast<uint32_t>(poolSize.Size);
                   });

    const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedLayoutInfo{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
        .bindingCount  = static_cast<uint32_t>(bindingFlags.size()),
        .pBindingFlags = bindingFlags.data()};

    const VkDescriptorSetLayoutCreateInfo bindlessLayoutInfo{
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = &extendedLayoutInfo,
        .flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings    = bindings.data()};

    const auto& vulkanRHI = vulkanContext.GetRHI();
    spdlog::trace("Creating Bindless descriptor set layout...");
    VK_ASSERT(vkCreateDescriptorSetLayout(vulkanRHI.GetDevice(), &bindlessLayoutInfo, nullptr, &bindlessLayout),
              "Failed to create bindless descriptor set layout.");

    spdlog::trace("Creating pool package...");
    VK_ASSERT(vkCreateDescriptorPool(vulkanRHI.GetDevice(), &poolCreateInfo, nullptr, &descriptorPoolPackage.DescriptorPool), "Failed to create descriptor pool.");

    const VkDescriptorSetVariableDescriptorCountAllocateInfoEXT descriptorSetVariableDescriptorCountAllocateInfo{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
        .pNext              = nullptr,
        .descriptorSetCount = 1,
        .pDescriptorCounts  = descriptorCounts.data()};

    const VkDescriptorSetAllocateInfo setAllocateInfo{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = &descriptorSetVariableDescriptorCountAllocateInfo,
        .descriptorPool     = descriptorPoolPackage.DescriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &bindlessLayout};

    VK_ASSERT(vkAllocateDescriptorSets(vulkanRHI.GetDevice(), &setAllocateInfo, &descriptorPoolPackage.DescriptorSet), "Failed to allocate descriptor set.");

    for (const auto& poolSize : poolSizes)
    {
        auto& offsetPoolPackage = descriptorPoolPackage.OffsetPoolPackages[ToUnderlying(poolSize.Type)];
        offsetPoolPackage.Pool.Grow(poolSize.Size);
        offsetPoolPackage.AllocatedSlots.resize(poolSize.Size);
    }
}

void DescriptorAllocator::Shutdown()
{
    spdlog::info("Shutdown Descriptor Manager.");
    const auto& vulkanRHI = vulkanContext.GetRHI();
    vkDestroyDescriptorPool(vulkanRHI.GetDevice(), descriptorPoolPackage.DescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(vulkanRHI.GetDevice(), bindlessLayout, nullptr);
}

void DescriptorAllocator::BeginFrame()
{
    auto& pendingList = pendingDeallocations[frameTracker.GetCurrentInFlightFrameIndex()];
    for (const Allocation& allocation : pendingList)
    {
        allocation.Owner.Pool.Deallocate(allocation.AllocatedSlot);
    }
    pendingList.clear();
}

void DescriptorAllocator::EndFrame()
{
    const bool bHasBufferDescriptorToUpdate = !bufferWriteDescriptors.empty();
    const bool bHasImageDescriptorToUpdate  = !imageWriteDescriptors.empty();
    if (bHasBufferDescriptorToUpdate || bHasImageDescriptorToUpdate)
    {
        combinedWriteDescriptorSets.reserve(bufferWriteDescriptors.size() + imageWriteDescriptors.size());
        if (bHasBufferDescriptorToUpdate)
        {
            for (size_t idx = 0; idx < bufferWriteDescriptors.size(); ++idx)
            {
                bufferWriteDescriptors[idx].pBufferInfo = &bufferInfos[idx];
            }

            combinedWriteDescriptorSets.assign(bufferWriteDescriptors.begin(), bufferWriteDescriptors.end());
        }

        if (bHasImageDescriptorToUpdate)
        {
            for (size_t idx = 0; idx < imageWriteDescriptors.size(); ++idx)
            {
                imageWriteDescriptors[idx].pImageInfo = &imageInfos[idx];
            }

            combinedWriteDescriptorSets.insert(combinedWriteDescriptorSets.end(), imageWriteDescriptors.begin(),
                                               imageWriteDescriptors.end());
        }

        if (!combinedWriteDescriptorSets.empty())
        {
            vkUpdateDescriptorSets(vulkanContext.GetRHI().GetDevice(),
                                   static_cast<uint32_t>(combinedWriteDescriptorSets.size()),
                                   combinedWriteDescriptorSets.data(), 0, nullptr);
            bufferWriteDescriptors.clear();
            bufferInfos.clear();
            imageWriteDescriptors.clear();
            imageInfos.clear();
            combinedWriteDescriptorSets.clear();
        }
    }
}

Descriptor DescriptorAllocator::RequestDescriptor(const vk::Buffer& buffer, const bool bIsDynamic)
{
    const auto descriptorType    = vk::BufferUsageToDescriptorType(buffer.GetUsage(), bIsDynamic);
    const auto descriptorBinding = ToUnderlying(descriptorType);
    auto&      offsetPoolPackage = descriptorPoolPackage.OffsetPoolPackages[descriptorBinding];

    auto&  allocatedSlots = offsetPoolPackage.AllocatedSlots;
    size_t slotOffset;
    {
        std::lock_guard               lock{offsetPoolPackage.Mutex};
        const FixedOffsetPool::Slot_t allocatedSlot = offsetPoolPackage.Pool.Allocate();
        slotOffset                                  = allocatedSlot.Offset;
        allocatedSlots[slotOffset]                  = allocatedSlot;
    }

    // Add new write descriptor set to write descriptor set list
    {
        std::lock_guard lock{bufferWriteDescriptorMutex};

        const VkWriteDescriptorSet writeDescriptorSet{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = GetDescriptorSet(),
            .dstBinding       = descriptorBinding,
            .dstArrayElement  = static_cast<uint32_t>(slotOffset),
            .descriptorCount  = 1,
            .descriptorType   = ToNative(descriptorType),
            .pImageInfo       = nullptr,
            .pTexelBufferView = nullptr};

        bufferInfos.emplace_back(buffer.GetDescriptorInfo());
        bufferWriteDescriptors.emplace_back(writeDescriptorSet);
    }

    return {
        &allocatedSlots[slotOffset],
        [this, &offsetPoolPackage](const FixedOffsetPool::Slot_t* slotPtr) {
            auto&           pendingList      = pendingDeallocations[frameTracker.GetCurrentInFlightFrameIndex()];
            auto&           pendingListMutex = pendingMutexList[frameTracker.GetCurrentInFlightFrameIndex()];
            std::lock_guard lock{pendingListMutex};
            pendingList.emplace_back(*slotPtr, offsetPoolPackage);
        }};
}

Descriptor DescriptorAllocator::RequestDescriptor(HandleManager& handleManager, const Handle<Buffer> handle, bool bIsDynamic)
{
    SY_ASSERT(handle, "Invalid Buffer Handle");
    if (!handle)
    {
        SY_ASSERT(false, "Invalid Buffer Resource");
        return nullptr;
    }

    return RequestDescriptor(*handle, bIsDynamic);
}

Descriptor DescriptorAllocator::RequestDescriptor(const vk::Texture& texture, const TextureView& view, const Sampler& sampler, const ETextureState expectedState, const bool bIsCombinedSampler)
{
    const auto descriptorType    = vk::ImageUsageToDescriptorType(texture.GetUsage(), bIsCombinedSampler);
    const auto descriptorBinding = ToUnderlying(descriptorType);
    auto&      offsetPoolPackage = descriptorPoolPackage.OffsetPoolPackages[descriptorBinding];

    auto&  allocatedSlots = offsetPoolPackage.AllocatedSlots;
    size_t slotOffset;
    {
        std::lock_guard               lock{offsetPoolPackage.Mutex};
        const FixedOffsetPool::Slot_t allocatedSlot = offsetPoolPackage.Pool.Allocate();
        slotOffset                                  = allocatedSlot.Offset;
        allocatedSlots[slotOffset]                  = allocatedSlot;
    }

    // Add new write descriptor set to write descriptor set list
    {
        std::lock_guard lock{imageWriteDescriptorMutex};

        const VkWriteDescriptorSet writeDescriptorSet{
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext            = nullptr,
            .dstSet           = GetDescriptorSet(),
            .dstBinding       = descriptorBinding,
            .dstArrayElement  = static_cast<uint32_t>(slotOffset),
            .descriptorCount  = 1,
            .descriptorType   = ToNative(descriptorType),
            .pImageInfo       = nullptr,
            .pTexelBufferView = nullptr};

        const auto [pipelineStage, accessFlag, layout] = QueryAccessPattern(expectedState);
        const VkDescriptorImageInfo descriptorImageInfo{
            .sampler     = sampler.GetNative(),
            .imageView   = view.GetNative(),
            .imageLayout = layout};
        imageInfos.emplace_back(descriptorImageInfo);
        imageWriteDescriptors.emplace_back(writeDescriptorSet);
    }

    return {
        &allocatedSlots[slotOffset],
        [this, &offsetPoolPackage](const FixedOffsetPool::Slot_t* slotPtr) {
            auto&           pendingList      = pendingDeallocations[frameTracker.GetCurrentInFlightFrameIndex()];
            auto&           pendingListMutex = pendingMutexList[frameTracker.GetCurrentInFlightFrameIndex()];
            std::lock_guard lock{pendingListMutex};
            pendingList.emplace_back(*slotPtr, offsetPoolPackage);
        }};
}

Descriptor DescriptorAllocator::RequestDescriptor(HandleManager& handleManager, const Handle<Texture> texture, const Handle<TextureView> view, const Handle<Sampler> sampler, const ETextureState expectedState, const bool bIsCombinedSampler)
{
    SY_ASSERT(texture, "Invalid Texture Handle.");
    SY_ASSERT(view, "Invalid Texture View Handle.");
    SY_ASSERT(sampler, "Invalid Sampler Handle.");

    if (!texture)
    {
        SY_ASSERT(false, "Invalid Texture Resource.");
        return nullptr;
    }

    if (!view)
    {
        SY_ASSERT(false, "Invalid Texture View Resource.");
        return nullptr;
    }

    if (!sampler)
    {
        SY_ASSERT(false, "Invalid Sampler Resource.");
        return nullptr;
    }

    return RequestDescriptor(*texture, *view, *sampler, expectedState, bIsCombinedSampler);
}

} // namespace vk
} // namespace sy
