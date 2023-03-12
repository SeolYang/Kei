#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanContext;
	class Buffer;
	class BufferBuilder
	{
		friend Buffer;

	public:
		BufferBuilder(const VulkanContext& vulkanContext) :
			vulkanContext(vulkanContext)
		{
		}

		~BufferBuilder() = default;

		BufferBuilder& SetName(const std::string& name)
		{
			this->name = name;
			return *this;
		}

		BufferBuilder& SetSize(const size_t newSizeAsBytes)
		{
			SY_ASSERT(newSizeAsBytes > 0, "Buffer size must be greater than 0.");
			this->size = newSizeAsBytes;
			return *this;
		}

		template <typename T>
		BufferBuilder& SetSize()
		{
			SetSize(sizeof(T));
			return *this;
		}

		BufferBuilder& SetTargetInitialState(const EBufferState state)
		{
			this->targetInitialState = state;
			return *this;
		}

		BufferBuilder& SetUsage(const VkBufferUsageFlags usage)
		{
			this->usage = usage;
			return *this;
		}

		BufferBuilder& SetMemoryUsage(const VmaMemoryUsage memoryUsage)
		{
			this->memoryUsage = memoryUsage;
			return *this;
		}

		template <typename T>
		BufferBuilder& SetDataToTransfer(const std::span<const T> typedData)
		{
			[[likely]]
			if (const bool bIsValidData = typedData.size() > 0 || typedData.data() != nullptr;
				bIsValidData)
			{
				const std::span<const uint8_t> bytesOfData = std::span<const uint8_t>{ reinterpret_cast<const uint8_t*>(typedData.data()), typedData.size_bytes() };
				dataToTransfer = bytesOfData;
			}
			else
			{
				dataToTransfer = std::nullopt;
			}

			return *this;
		}

		template <typename T>
		BufferBuilder& SetDataToTransferWithSize(const std::span<const T> typedData)
		{
			return SetDataToTransfer(typedData).SetSize(typedData.size_bytes());
		}

		[[nodiscard]] bool IsValidToBuild() const { return (size > 0) && (usage.has_value()) && (memoryUsage.has_value()); }

		[[nodiscard]] std::unique_ptr<Buffer> Build() const;

	private:
		const VulkanContext& vulkanContext;
		std::string name = "Buffer";
		size_t size = 1;
		EBufferState targetInitialState = EBufferState::None;
		/** @todo May builder have vector of bytes instead of span? cause it can be dangling in some situation. */
		std::optional<std::span<const uint8_t>> dataToTransfer = std::nullopt; 
		std::optional<VkBufferUsageFlags> usage = std::nullopt;
		std::optional<VmaMemoryUsage> memoryUsage = std::nullopt;

	};


}
