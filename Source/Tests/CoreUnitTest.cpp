#include <PCH.h>
#include <catch.hpp>
#include <Core/Utils.h>
#include <Core/HandleManager.h>

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

		extent = {4096,
                  4096,
                  2048};
        REQUIRE(CalculateMaximumMipCountFromExtent(extent) == 13);
    }
}

TEST_CASE("HandleMap", "[handle_map]")
{
    SECTION("Allocation of Handle")
    {
        sy::HandleMap<size_t> map;
        auto                  handleOfHundred = map.Add(std::make_unique<size_t>(100));
        REQUIRE(handleOfHundred.IsValid());
        REQUIRE(!handleOfHundred.HasAlias());
        REQUIRE(handleOfHundred.TryGetObject() != std::nullopt);
        REQUIRE(handleOfHundred.TryGetAlias() == std::nullopt);
        REQUIRE(*handleOfHundred == 100);

        auto handleOfTwo = map.Add(2);
        REQUIRE(handleOfTwo.IsValid());
        REQUIRE(!handleOfTwo.HasAlias());
        REQUIRE(*handleOfTwo == 2);

        REQUIRE(handleOfHundred.GetPlacement() != handleOfTwo.GetPlacement());

        auto emptyHandle = sy::Handle<size_t>{};
        REQUIRE(!emptyHandle.IsValid());
        REQUIRE(emptyHandle.TryGetObject() == std::nullopt);
        REQUIRE(emptyHandle.TryGetAlias() == std::nullopt);
        REQUIRE(emptyHandle.GetPlacement() == sy::InvalidPlacement);

        auto invalidHandle = map.Query(1231234); /* Querying non-existing handle */
        REQUIRE(!invalidHandle.IsValid());
        REQUIRE(invalidHandle.TryGetObject() == std::nullopt);
        REQUIRE(invalidHandle.GetPlacement() == sy::InvalidPlacement);

        handleOfHundred.DestroySelf();
        REQUIRE(!handleOfHundred.IsValid());
    }

    SECTION("Handle Alias")
    {
        sy::HandleMap<size_t>      map;
        constexpr std::string_view HundredAlias = "Hundred";

        auto handleOfHundred = map.Add(std::make_unique<size_t>(100));
        handleOfHundred.SetAlias(HundredAlias);
        REQUIRE(handleOfHundred.GetAlias() == HundredAlias);

        auto anotherHandleOfHundred = map.QueryAlias(HundredAlias);
        REQUIRE(handleOfHundred.GetPlacement() == anotherHandleOfHundred.GetPlacement());
        REQUIRE(*anotherHandleOfHundred == 100);

        constexpr std::string_view RenamedHundredAlias = "PrettryHundred";
        handleOfHundred.SetAlias(RenamedHundredAlias);
        REQUIRE(handleOfHundred.GetAlias() == RenamedHundredAlias);
        REQUIRE(anotherHandleOfHundred.GetAlias() == RenamedHundredAlias);

        auto handleOfHundredAfterRenamed = map.QueryAlias(RenamedHundredAlias);
        REQUIRE(handleOfHundredAfterRenamed.IsValid());
        REQUIRE(handleOfHundred.GetPlacement() == handleOfHundredAfterRenamed.GetPlacement());

        auto queryFromBeforeAlias = map.QueryAlias(HundredAlias);
        REQUIRE(!queryFromBeforeAlias.IsValid());

        auto invalidHandle = map.Query(123131313);
        invalidHandle.SetAlias("Invalid");
        REQUIRE(!invalidHandle.IsValid());
        REQUIRE(invalidHandle.TryGetAlias() == std::nullopt);
    }
}

TEST_CASE("HandleManager", "[handle_mng]")
{
    using HandleManager = sy::HandleManager;
    SECTION("Allocation of HandleMap")
    {
        constexpr float PI = 3.141592f;
        HandleManager   handleMng;
        auto&           floatHandleMap = handleMng.GetHandleMap<float>();
        auto            piHandle       = handleMng.Add<float>(PI);
        REQUIRE(piHandle.IsValid());
        const auto ownerOfPiHandle = piHandle.GetOwner();
        REQUIRE(&floatHandleMap == &ownerOfPiHandle.value().get());
        auto anotherHandleForPi = handleMng.Query<float>(piHandle.GetPlacement());
        REQUIRE(anotherHandleForPi.IsValid());
        REQUIRE(anotherHandleForPi.GetPlacement() == piHandle.GetPlacement());

        REQUIRE((*piHandle == *anotherHandleForPi));

        piHandle.SetAlias("PI");
        const auto piHandleFromAlias = handleMng.QueryAlias<float>("PI");
        REQUIRE(piHandleFromAlias.GetPlacement() == piHandle.GetPlacement());
        REQUIRE(piHandle.GetAlias() == "PI");

        const auto piHandleWithAliasFromOtherType = handleMng.QueryAlias<int>("PI");
        REQUIRE(!piHandleWithAliasFromOtherType.IsValid());
    }
}

TEST_CASE("Utilities", "[utils]")
{
    SECTION("Flags")
    {
        constexpr VkCommandBufferUsageFlags tempFlagBits = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        REQUIRE(sy::ContainsBitFlag<VkCommandBufferUsageFlags>(tempFlagBits, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));
    }
}
