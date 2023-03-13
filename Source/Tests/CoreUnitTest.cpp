#include <PCH.h>
#include <catch.hpp>
#include <Core/ResourceCache.h>
#include <Core/Utils.h>

TEST_CASE("Extent2D", "[extent_2d]")
{
	SECTION("Validation Check")
	{
		sy::Extent2D<uint32_t> extent{0, 1};
		REQUIRE((extent.width == 0 && extent.height == 1));
		REQUIRE(!extent.IsValid());

		extent = {100, 100};
		REQUIRE((extent.width == 100 && extent.height == 100));
		REQUIRE(extent.IsValid());
	}
}

TEST_CASE("Extent3D", "[extent_3d]")
{
	SECTION("Validation Check")
	{
		sy::Extent3D<uint32_t> extent{0, 0, 1};
		REQUIRE((extent.width == 0 && extent.height == 0 && extent.depth == 1));
		REQUIRE(!extent.IsValid());

		extent = {100, 200, 0};
		REQUIRE((extent.width == 100 && extent.height == 200 && extent.depth == 0));
		REQUIRE(!extent.IsValid());

		extent = {300, 400, 500};
		REQUIRE((extent.width == 300 && extent.height == 400 && extent.depth == 500));
		REQUIRE(extent.IsValid());
	}
}

TEST_CASE("ResourceCache", "[res_cache]")
{
	SECTION("Resource caching")
	{
		const auto resCache = std::make_unique<sy::ResourceCache>();
		const auto newResource = resCache->Add<uint32_t>(153);
		REQUIRE(newResource.IsValidHandleValue());
		REQUIRE(resCache->Contains(newResource));

		const auto anotherResource = resCache->Add<uint32_t>(153);
		REQUIRE(anotherResource.IsValidHandleValue());
		REQUIRE(resCache->Contains(anotherResource));
		REQUIRE(newResource.Value != anotherResource.Value);

		const auto otherTypeNewResource = resCache->Add<float>(153.f);
		REQUIRE(otherTypeNewResource.IsValidHandleValue());
	}

	SECTION("Load resource from cache")
	{
		/** @todo Handle<T> is only used for a resource cache, so its better to specialize it for that specific purpose. */
		const auto resCache = std::make_unique<sy::ResourceCache>();
		const auto newResource = resCache->Add<uint32_t>(153);
		const auto newResourceValOpt = resCache->Load(newResource);
		REQUIRE(newResourceValOpt.has_value());
		REQUIRE(*newResourceValOpt == 153);

		/** @todo Prevent Manually created handle! */
		constexpr sy::Handle<uint32_t> manualHandle{1};
		const auto manualValOpt = resCache->Load(manualHandle);
		REQUIRE(manualValOpt.has_value());
		REQUIRE(*manualValOpt == 153);

		const auto anotherResource = resCache->Add<uint32_t>(303);
		const auto anotherResourceValOpt = resCache->Load(anotherResource);
		REQUIRE(anotherResourceValOpt.has_value());

		constexpr sy::Handle<uint32_t> invalidHandle{ 30303};
		REQUIRE(!resCache->Load(invalidHandle).has_value());
	}

	SECTION("Aliasing")
	{
		const auto resCache = std::make_unique<sy::ResourceCache>();
		const auto newResource = resCache->Add<uint32_t>(153);
		const auto anotherResource = resCache->Add<uint32_t>(435);
		REQUIRE(resCache->SetAlias("NiceInteger", newResource));
		REQUIRE(resCache->Contains<uint32_t>("NiceInteger"));
		REQUIRE(resCache->SetAlias("153", newResource));
		REQUIRE(resCache->Contains<uint32_t>("NiceInteger"));
		REQUIRE(resCache->Contains<uint32_t>("153"));
		REQUIRE(resCache->Load<uint32_t>("NiceInteger").value() == resCache->Load<uint32_t>("153").value());
	}
}

TEST_CASE("Utilities", "[utils]")
{
	SECTION("Flags")
	{
		constexpr VkCommandBufferUsageFlags tempFlagBits = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		REQUIRE(sy::FlagsContains<VkCommandBufferUsageFlags>(tempFlagBits, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));
	}
}