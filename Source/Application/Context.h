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
}

namespace sy::render
{
	class Renderer;
}

namespace sy::game
{
	class World;
	class GameContext;
}

namespace sy::window
{
	class Window;
}

namespace sy::app
{
	class Context final : public NonCopyable
	{
	public:
		Context(int argc, char** argv);
		virtual ~Context() override;

		void Run();

	private:
		void Startup(int argc, char** argv);
		void InitializeLogger();
		void InitializeCommandLineParser(int argc, char** argv);
		void InitDefaultEngineResources();

		void Cleanup();

	private:
		std::unique_ptr<CommandLineParser> cmdLineParser;
		std::unique_ptr<Timer> timer;
		std::unique_ptr<window::Window> window;
		std::unique_ptr<HandleManager> handleManager;
		std::unique_ptr<vk::VulkanContext> vulkanContext;
		std::unique_ptr<vk::ResourceStateTracker> resourceStateTracker;
		std::unique_ptr<render::Renderer> renderer;
		std::unique_ptr<game::World> world;
		std::unique_ptr<game::GameContext> gameContext;
	};
}
