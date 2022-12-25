#include <Core/Core.h>
#include <Core/Context.h>
#include <Render/RenderGraph.h>

int main(int argc, char** argv)
{
	using namespace sy;
	Context context {argc, argv};
	context.Run();
	return 0;
}