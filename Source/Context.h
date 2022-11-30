#pragma once
#include <Core.h>

namespace sy
{
	class Window;
	class VulkanContext;
	class FrameTracker;
	class CommandPoolManager;
	class DescriptorManager;
	class Renderer;
	class GameInstance;
	class Context
	{
	public:
		Context();
		Context(const Context&) = delete;
		Context(Context&&) = default;

		~Context();

		Context& operator=(const Context&) = delete;
		Context& operator=(Context&&) = default;

		void Run();

	private:
		void Startup();
		void Cleanup();

	private:
		std::unique_ptr<Timer> timer;
		std::unique_ptr<Window> window;
		std::unique_ptr<VulkanContext> vulkanContext;
		std::unique_ptr<FrameTracker> frameTracker;
		std::unique_ptr<CommandPoolManager> cmdPoolManager;
		std::unique_ptr<DescriptorManager> descriptorManager;
		std::unique_ptr<Renderer> renderer;
		std::unique_ptr<GameInstance> gameInstance;

	};
}