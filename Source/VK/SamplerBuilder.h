#pragma once
#include <PCH.h>

namespace sy::vk
{
	class Sampler;

	class SamplerBuilder
	{
	public:
		explicit SamplerBuilder(const VulkanContext& vulkanContext) :
			vulkanContext(vulkanContext)
		{
		}

		SamplerBuilder& SetName(const std::string_view name)
		{
			this->name = name;
			return *this;
		}

		SamplerBuilder& SetMinFilter(const VkFilter minFilter)
		{
			this->minFilter = minFilter;
			return *this;
		}

		SamplerBuilder& SetMagFilter(const VkFilter magFilter)
		{
			this->magFilter = magFilter;
			return *this;
		}

		SamplerBuilder& SetMipmapMode(const VkSamplerMipmapMode mode)
		{
			this->mipmapMode = mode;
			return *this;
		}

		SamplerBuilder& SetAddressModeU(const VkSamplerAddressMode mode)
		{
			this->addressModeU = mode;
			return *this;
		}

		SamplerBuilder& SetAddressModeV(const VkSamplerAddressMode mode)
		{
			this->addressModeV = mode;
			return *this;
		}

		SamplerBuilder& SetAddressModeW(const VkSamplerAddressMode mode)
		{
			this->addressModeW = mode;
			return *this;
		}

		SamplerBuilder& SetAddressMode(const VkSamplerAddressMode mode)
		{
			this->addressModeU = mode;
			this->addressModeV = mode;
			this->addressModeW = mode;
			return *this;
		}

		std::unique_ptr<Sampler> Build() const;

	private:
		friend class Sampler;
		const VulkanContext& vulkanContext;
		std::string name                  = "LinearRepeatSampler";
		VkFilter minFilter                = VK_FILTER_LINEAR;
		VkFilter magFilter                = VK_FILTER_LINEAR;
		VkSamplerMipmapMode mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	};
}
