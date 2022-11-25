#include <Core.h>
#include <Instance.h>
#include <Window.h>
#include <VK/VulkanInstance.h>
#include <Renderer.h>
#include <GameInstance.h>

namespace sy
{
	Instance::Instance()
	{
		Startup();
	}

	Instance::~Instance()
	{
		Cleanup();
	}

	void Instance::Startup()
	{
#if defined(_DEBUG) || defined(DEBUG)
		spdlog::set_level(spdlog::level::trace);
#elif
		spdlog::set_level(spdlog::level::warn);
#endif
		timer = std::make_unique<Timer>();
		window = std::make_unique<Window>("Test", Extent2D<uint32_t>{ 1280, 720 });
		vulkanInstance = std::make_unique<VulkanInstance>(*window);
		renderer = std::make_unique<Renderer>(*window, *vulkanInstance);
	}

	void Instance::Cleanup()
	{
		renderer.reset();
		vulkanInstance.reset();
		window.reset();
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

			renderer->Render();

			timer->End();
        }
	}
}