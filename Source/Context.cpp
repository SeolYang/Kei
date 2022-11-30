#include <Core.h>
#include <Context.h>
#include <Window.h>
#include <VK/VulkanContext.h>
#include <FrameTracker.h>
#include <CommandPoolManager.h>
#include <DescriptorManager.h>
#include <Renderer.h>
#include <GameInstance.h>

namespace sy
{
	Context::Context()
	{
		Startup();
	}

	Context::~Context()
	{
		Cleanup();
	}

	void Context::Startup()
	{
#if defined(_DEBUG) || defined(DEBUG)
		spdlog::set_level(spdlog::level::trace);
#else
		spdlog::set_level(spdlog::level::warn);
#endif
		timer = std::make_unique<Timer>();
		window = std::make_unique<Window>("Test", Extent2D<uint32_t>{ 1280, 720 });
		vulkanContext = std::make_unique<VulkanContext>(*window);
		frameTracker = std::make_unique<FrameTracker>(*vulkanContext);
		cmdPoolManager = std::make_unique<CommandPoolManager>(*vulkanContext, *frameTracker);
		descriptorManager = std::make_unique<DescriptorManager>(*vulkanContext, *frameTracker);
		renderer = std::make_unique<Renderer>(*window, *vulkanContext, *frameTracker, *cmdPoolManager, *descriptorManager);
	}

	void Context::Cleanup()
	{
		vulkanContext->WaitForDeviceIdle();
		{
			renderer.reset();
			descriptorManager.reset();
			cmdPoolManager.reset();
			frameTracker.reset();
			vulkanContext.reset();
		}
		window.reset();
	}

	void Context::Run()
	{
        SDL_Event ev;
        bool bExit = false;

        while (!bExit)
        {
			timer->Begin();
			frameTracker->BeginFrame();

            while (SDL_PollEvent(&ev) != 0)
            {
                if (ev.type == SDL_QUIT)
                {
                    bExit = true;
                }
            }

			frameTracker->WaitForInFlightRenderFence();
			cmdPoolManager->BeginFrame();
			descriptorManager->BeginFrame();
			renderer->Render();
			descriptorManager->EndFrame();
			cmdPoolManager->EndFrame();

			frameTracker->EndFrame();
			timer->End();
        }
	}
}