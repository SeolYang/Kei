#pragma once
#include <PCH.h>

namespace sy
{
	class CommandLineParser;
}

namespace sy::vk
{
	class VulkanContext;
	class ResourceStateTracker;
} // namespace sy::vk

namespace sy::render
{
	class Renderer;
}

namespace sy::game
{
} // namespace sy::game

namespace sy::window
{
	class Window;
}

namespace sy::app
{
	class Context final : public NonCopyable
	{
	public:
		Context(CommandLineParser& cmdLineParser, window::Window& window);
		~Context() override;

		void Run();

		void Startup();
		void Shutdown();

	private:
		void InitializeLogger();
		void InitDefaultEngineResources();

	private:
		CommandLineParser& cmdLineParser;
		window::Window& window;
		std::unique_ptr<Timer> timer;
		std::unique_ptr<HandleManager> handleManager;
		std::unique_ptr<vk::VulkanContext> vulkanContext;
		std::unique_ptr<render::Renderer> renderer;
	};
} // namespace sy::app
