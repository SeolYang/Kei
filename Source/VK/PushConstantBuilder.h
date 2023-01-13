#pragma once
#include <PCH.h>

namespace sy::vk
{
	class PushConstantBuilder : public NonCopyable
	{
	public:
		PushConstantBuilder() = default;
		~PushConstantBuilder() override = default;

		template <typename T>
		PushConstantBuilder& Add(const VkShaderStageFlags shaderStage)
		{
			ranges.emplace_back(shaderStage, offset, sizeof(T));
			offset += sizeof(T);
			return *this;
		}

		std::span<const VkPushConstantRange> Build() const
		{
			return ranges;
		}

	private:
		std::vector<VkPushConstantRange> ranges;
		size_t offset = 0;

	};
}