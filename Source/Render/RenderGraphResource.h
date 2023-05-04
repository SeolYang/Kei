#pragma once
#include <PCH.h>
#include <VK/TextureBuilder.h>
#include <VK/BufferBuilder.h>

namespace sy::vk
{
class VulkanContext;
class Texture;
class Buffer;
} // namespace sy::vk

namespace sy::render
{
template <typename T, typename BuilderType, typename StateType>
class RenderGraphResource : public NonCopyable
{
public:
    RenderGraphResource(vk::VulkanContext& vulkanContext, const std::string_view name) :
        name(name),
        builder(BuilderType(vulkanContext))
    {
    }

    ~RenderGraphResource() = default;

    T& Instantiate()
    {
        if (instance == nullptr)
        {
            instance = builder.Build();
        }

        return *instance;
    }

	void WriteBy(const std::string writerName)
	{
        SY_ASSERT(!HasWriter(), "Resource {} already created by node:{}", name, writer);
        writer = writerName;
	}

    void ReadBy(const std::string_view readerName, const StateType state)
    {
		SY_ASSERT(!IsWriteBy(readerName), "Self write-read dependency accurs at resource {}.", name);
        SY_ASSERT(!IsReadBy(readerName), "Resource {} already read by {}.", name, readerName);
        readers.insert(readerName.data());
        readerStateMap[readerName.data()] = state;
    }

	[[nodiscard]] std::string_view GetWriter() const { return writer; }
    [[nodiscard]] bool HasWriter() const { return !writer.empty(); }
    [[nodiscard]] bool IsWriteBy(const std::string_view writerName) const { return writerName == writer; }
    [[nodiscard]] bool IsReadBy(const std::string_view readerName) const { return readers.contains(readerName.data()); }
    [[nodiscard]] std::string_view GetName() const { return name; }
    [[nodiscard]] BuilderType& GetBuilder() { return builder; }
    [[nodiscard]] const auto& GetReaders() const { return readers; }
    [[nodiscard]] auto QueryStateFor(const std::string_view reader) const { return readerStateMap.at(reader.data()); }

private:
    const std::string name;
    std::unique_ptr<T> instance = nullptr;
    BuilderType builder;
    std::string writer;
    robin_hood::unordered_set<std::string> readers;
    robin_hood::unordered_map<std::string, StateType> readerStateMap;
};


using RenderGraphTexture = RenderGraphResource<vk::Texture, vk::TextureBuilder, vk::ETextureState>;
using RenderGraphBuffer = RenderGraphResource<vk::Buffer, vk::BufferBuilder, vk::EBufferState>;
} // namespace sy::render
