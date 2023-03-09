#include <PCH.h>
#include <Application/Context.h>

#include <Asset/ModelAsset.h>

int main(int argc, char** argv)
{
	using namespace sy;
	app::Context context {argc, argv};
	context.Run();
	return 0;
}