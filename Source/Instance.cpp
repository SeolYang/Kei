#include <Core.h>
#include <Instance.h>
#include <Window.h>
#include <GameInstance.h>

namespace sy
{
	Instance::Instance()
	{
		timer = std::make_unique<Timer>();
		window = std::make_unique<Window>("Test", Extent2D<uint32_t>{ 1280, 720 });
	}

	Instance::~Instance()
	{
		window->Cleanup();
	}

	void Instance::Startup()
	{
		timer->Startup();
		window->Startup();
	}

	void Instance::Run()
	{
        SDL_Event ev;
        bool bExit = false;

        while (!bExit)
        {
			timer->Begin();

            while (SDL_PollEvent(&ev) != 0)
            {
                if (ev.type == SDL_QUIT)
                {
                    bExit = true;
                }
            }

			timer->End();
        }
	}
}