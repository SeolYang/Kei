#include <PCH.h>
#include <Application/Context.h>

#include <Asset/ModelAsset.h>
#include <catch.hpp>

int main(int argc, char** argv)
{
	bool bUnitTestRequired = false;
	Catch::Session session;
	const auto cli = session.cli() | Catch::Clara::Opt(bUnitTestRequired)[ "--test" ]("Execute Unit Tests.");
	session.cli(cli);

	if (const int returnCode = session.applyCommandLine(argc, argv); returnCode != 0)
	{
		return returnCode;
	}

	if (bUnitTestRequired)
	{
		const auto numOfFailedTestCase = session.run();
		return numOfFailedTestCase;
	}

	using namespace sy;
	app::Context context{ argc, argv };
	context.Run();

	return 0;
}
