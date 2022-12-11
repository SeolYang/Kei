#include <Core/Core.h>
#include <Core/Context.h>

int main(int argc, char** argv)
{
	using namespace sy;
	sy::Context contextInstance{ argc, argv };
	contextInstance.Run();
	return 0;
}