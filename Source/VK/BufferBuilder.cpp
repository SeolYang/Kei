#include <PCH.h>
#include <VK/BufferBuilder.h>
#include <VK/Buffer.h>

namespace sy::vk
{
	std::unique_ptr<Buffer> BufferBuilder::Build() const
	{
		SY_ASSERT(IsValidToBuild(), "Buffer builder is not valid to build.");
		return std::make_unique<Buffer>(*this);
	}
}
