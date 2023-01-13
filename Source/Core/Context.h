#pragma once
#include <PCH.h>

namespace sy
{
	namespace vk
	{
		class VulkanContext;
		class FrameTracker;
		class CommandPoolManager;
		class DescriptorManager;
	}

	namespace render
	{
		class Renderer;
	}

	namespace game
	{
		class World;
		class GameContext;
	}

	class CommandLineParser;
	class Window;
	class CacheRegistry;
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

		void Cleanup();

	private:
		std::unique_ptr<CommandLineParser> cmdLineParser;
		std::unique_ptr<Timer> timer;
		std::unique_ptr<Window> window;
		std::unique_ptr<CacheRegistry> cacheRegistry;
		std::unique_ptr<vk::VulkanContext> vulkanContext;
		std::unique_ptr<vk::FrameTracker> frameTracker;
		std::unique_ptr<vk::CommandPoolManager> cmdPoolManager;
		std::unique_ptr<vk::DescriptorManager> descriptorManager;
		std::unique_ptr<render::Renderer> renderer;
		std::unique_ptr<game::World> world;
		std::unique_ptr<game::GameContext> gameContext;

	};
}