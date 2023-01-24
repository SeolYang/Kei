#include <PCH.h>
#include <Core/Context.h>

#include <Asset/ModelAsset.h>

int main(int argc, char** argv)
{
	using namespace sy;
	Context context {argc, argv};
	context.Run();
	return 0;
}