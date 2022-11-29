#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class DescriptorManager
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

			auto Build() const
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

			auto BuildAsNative() const
			{
				const auto temp = Build();
				std::vector<VkDescriptorPoolSize> nativeTemp;
				nativeTemp.resize(temp.size());
				std::transform(
					temp.cbegin(), temp.cend(),
					nativeTemp.begin(),
					[](const DescriptorPoolSize& val)
					{
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
			OffsetPool::Slot_t AllocatedSlot;
			OffsetPool& Owner;
		};

	public:
		DescriptorManager(const VulkanInstance& vulkanInstance);
		~DescriptorManager();

		void BeginFrame(size_t currentInFlightFrameIdx);
		void EndFrame(size_t currentInFlightFrameIdx);

		UniqueHandle<Allocation> RequestForTransientBuffer(size_t currentInFlightFrameIdx, EDescriptorType descriptorType);
		UniqueHandle<Allocation> RequestForTransientImage(size_t currentInFlightFrameIdx, EDescriptorType descriptorType);

		UniqueHandle<Allocation> RequestForPersistentBuffer(EDescriptorType descriptorType);
		UniqueHandle<Allocation> RequestForPersistentImage(EDescriptorType descriptorType);

		[[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const { return bindlessLayout; }

	private:
		const VulkanInstance& vulkanInstance;
		VkDescriptorSetLayout bindlessLayout = VK_NULL_HANDLE;
		std::array<PoolPackage, NumMaxInFlightFrames> transientDescriptorPackages;
		std::array<std::vector<Allocation>, NumMaxInFlightFrames> pendingTransientDeallocationLists;

		PoolPackage persistentDescriptorPackage;
		std::vector<Allocation> pendingPersistentDeallocations;

	};
}