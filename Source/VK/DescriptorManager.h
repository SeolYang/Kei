#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanRHI;
	class Buffer;
	class Texture;
	class TextureView;
	class Sampler;
	class FrameTracker;

	class DescriptorManager final : public NonCopyable
	{
	public:
		struct DescriptorPoolSize
		{
			const EDescriptorType Type;
			const size_t Size;
		};

		struct DescriptorPoolSizeBuilder
		{
		public:
			DescriptorPoolSizeBuilder()
			{
				for (auto& poolSize : poolSizes)
				{
					poolSize = 0;
				}
			}

			DescriptorPoolSizeBuilder& AddDescriptors(const EDescriptorType type, const size_t count)
			{
				descriptorCount += count;
				poolSizes[ToUnderlying(type)] += count;
				return *this;
			}

			[[nodiscard]] auto Build() const
			{
				std::vector<DescriptorPoolSize> temp;
				temp.reserve(poolSizes.size());
				for (size_t idx = 0; idx < poolSizes.size(); ++idx)
				{
					if (poolSizes[idx] > 0)
					{
						temp.emplace_back(static_cast<EDescriptorType>(idx), poolSizes[idx]);
					}
				}

				return temp;
			}

			[[nodiscard]] auto BuildAsNative() const
			{
				const auto temp = Build();
				std::vector<VkDescriptorPoolSize> nativeTemp;
				nativeTemp.resize(temp.size());
				std::transform(
					temp.cbegin(), temp.cend(),
					nativeTemp.begin(),
					[](const DescriptorPoolSize& val) {
						return VkDescriptorPoolSize{ ToNative(val.Type), static_cast<uint32_t>(val.Size) };
					});

				return nativeTemp;
			}

			[[nodiscard]] size_t GetDescriptorCount() const
			{
				return descriptorCount;
			}

		private:
			size_t descriptorCount = 0;
			std::array<size_t, ToUnderlying(EDescriptorType::EnumMax)> poolSizes;
		};

		struct OffsetPoolPackage
		{
			FixedOffsetPool Pool;
			std::vector<FixedOffsetPool::Slot_t> AllocatedSlots;
			std::mutex Mutex;
		};

		struct PoolPackage
		{
			VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
			VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
			std::array<OffsetPoolPackage, ToUnderlying(EDescriptorType::EnumMax)> OffsetPoolPackages;
		};

		struct Allocation
		{
			FixedOffsetPool::Slot_t AllocatedSlot;
			OffsetPoolPackage& Owner;
		};

	public:
		DescriptorManager(const VulkanRHI& vulkanRHI, const FrameTracker& frameTracker);
		virtual ~DescriptorManager() override;

		void BeginFrame();
		void EndFrame();

		[[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const
		{
			return bindlessLayout;
		}

		[[nodiscard]] VkDescriptorSet GetDescriptorSet() const
		{
			return descriptorPoolPackage.DescriptorSet;
		}

		Descriptor RequestDescriptor(const Buffer& buffer, bool bIsDynamic = false);
		Descriptor RequestDescriptor(HandleManager& handleManager, Handle<Buffer> handle, bool bIsDynamic = false);
		Descriptor RequestDescriptor(const Texture& texture, const TextureView& view, const Sampler& sampler,
			ETextureState expectedState, bool bIsCombinedSampler = true);
		Descriptor RequestDescriptor(HandleManager& handleManager, Handle<Texture> texture, Handle<TextureView> view,
			Handle<Sampler> sampler, ETextureState expectedState,
			bool bIsCombinedSampler = true);

	private:
		const VulkanRHI& vulkanRHI;
		const FrameTracker& frameTracker;
		VkDescriptorSetLayout bindlessLayout = VK_NULL_HANDLE;
		PoolPackage descriptorPoolPackage;
		std::array<std::vector<Allocation>, NumMaxInFlightFrames> pendingDeallocations;
		std::array<std::mutex, NumMaxInFlightFrames> pendingMutexList;

		std::vector<VkWriteDescriptorSet> combinedWriteDescriptorSets;
		std::vector<VkWriteDescriptorSet> imageWriteDescriptors;
		std::vector<VkDescriptorImageInfo> imageInfos;
		std::mutex imageWriteDescriptorMutex;
		std::vector<VkWriteDescriptorSet> bufferWriteDescriptors;
		std::vector<VkDescriptorBufferInfo> bufferInfos;
		std::mutex bufferWriteDescriptorMutex;
	};
} // namespace sy::vk
