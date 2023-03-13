#include <PCH.h>
#include <catch.hpp>

TEST_CASE("add function", "[add]")
{
	SECTION("Simple add")
	{
		REQUIRE(1 + 1 == 2);
	}
}
