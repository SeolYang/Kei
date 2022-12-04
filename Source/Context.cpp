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
		spdlog::info("Initializing Timer sub-context.");
		timer = std::make_unique<Timer>();
		spdlog::info("Initializing Window sub-context.");
		window = std::make_unique<Window>("Test", Extent2D<uint32_t>{ 1280, 720 });
		spdlog::info("Initializing Vulkan context.");
		vulkanContext = std::make_unique<VulkanContext>(*window);
		spdlog::info("Initializing frame tracker sub-context.");
		frameTracker = std::make_unique<FrameTracker>(*vulkanContext);
		spdlog::info("Initializing Command Pool Manager sub-context.");
		cmdPoolManager = std::make_unique<CommandPoolManager>(*vulkanContext, *frameTracker);
		spdlog::info("Initializing Bind-less Descriptor Manager sub-context.");
		descriptorManager = std::make_unique<DescriptorManager>(*vulkanContext, *frameTracker);
		spdlog::info("Initializing Renderer sub-context.");
		renderer = std::make_unique<Renderer>(*window, *vulkanContext, *frameTracker, *cmdPoolManager, *descriptorManager);
	}

	void Context::Cleanup()
	{
		vulkanContext->WaitForDeviceIdle();
		{
			spdlog::info("Clean-up Renderer sub-context.");
			renderer.reset();
			spdlog::info("Clean-up Bind-less Descriptor Manager sub-context.");
			descriptorManager.reset();
			spdlog::info("Clean-up Command Pool Manager sub-context.");
			cmdPoolManager.reset();
			spdlog::info("Clean-up frame tracker sub-context.");
			frameTracker.reset();
			spdlog::info("Clean-up Vulkan context.");
			vulkanContext.reset();
		}
		spdlog::info("Clean-up Renderer sub-context");
		window.reset();
	}

	void Context::Run()
	{
		spdlog::info("Startup main loop.");
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
		spdlog::info("Main loop finished.");
	}
}