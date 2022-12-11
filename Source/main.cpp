#include <Core/Core.h>
#include <Core/Context.h>
#include <Asset/AssetConverter.h>

int main(int, char**)
{
	using namespace sy;
	//asset::convert::ConvertTexture2D("Assets/Textures/djmax_1st_anv.png");
	//asset::ConvertAssets("Assets");
	sy::Context contextInstance;
	contextInstance.Run();
	return 0;
}