#include <Instance.h>
#include <ECS.h>

int main(int, char**)
{
	sy::Instance instance;
	instance.Startup();
	instance.Run();
	return 0;
}