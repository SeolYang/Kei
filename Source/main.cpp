#include <PCH.h>
#include <Application/Context.h>
#include <Core/CommandLineParser.h>
#include <Window/WindowBuilder.h>

#include <catch.hpp>

int main(int argc, char** argv)
{
    bool           bUnitTestRequired = false;
    Catch::Session session;
    const auto     cli =
        session.cli() |
        Catch::Clara::Opt(bUnitTestRequired)["--test"]("Execute Unit Tests.");
    session.cli(cli);

    if (const int returnCode = session.applyCommandLine(argc, argv);
        returnCode != 0)
    {
        return returnCode;
    }

    if (bUnitTestRequired)
    {
        const auto numOfFailedTestCase = session.run();
        return numOfFailedTestCase;
    }

    using namespace sy;
    const auto   cmdLineParser = std::make_unique<CommandLineParser>(argc, argv);
    app::Context context{*cmdLineParser, window::WindowBuilder{}};

    context.Startup();
    context.Run();
    context.Shutdown();

    return 0;
}
