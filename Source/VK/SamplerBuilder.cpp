#include <PCH.h>
#include <VK/SamplerBuilder.h>
#include <VK/Sampler.h>

namespace sy::vk
{
	std::unique_ptr<Sampler> SamplerBuilder::Build() const
	{
		return std::make_unique<Sampler>(*this);
	}
}
