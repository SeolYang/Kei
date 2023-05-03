#include <PCH.h>
#include <catch.hpp>
#include <VK/Texture.h>
#include <VK/Buffer.h>
#include <VK/Synchronization.h>

TEST_CASE("VK/Synchronization")
{
    using namespace sy::vk;
    SECTION("Overlapping AccessPattern")
    {
        auto accessPattern0 = QueryAccessPattern(ETextureState::ComputeShaderReadSampledImage);
        REQUIRE(accessPattern0.Access == VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        REQUIRE(accessPattern0.ImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        REQUIRE(accessPattern0.PipelineStage == VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);

        auto accessPattern1 = QueryAccessPattern(ETextureState::FragmentShaderReadSampledImage);
        REQUIRE(accessPattern1.Access == VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        REQUIRE(accessPattern1.ImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        REQUIRE(accessPattern1.PipelineStage == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

        accessPattern0.Overlap(accessPattern1);
        REQUIRE(accessPattern0.Access == VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        REQUIRE(accessPattern0.ImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        REQUIRE(accessPattern0.PipelineStage == (VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT));
    }
}