#pragma once
#include <Core/Core.h>

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

	class Window;
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
		std::unique_ptr<vk::VulkanContext> vulkanContext;
		std::unique_ptr<vk::FrameTracker> frameTracker;
		std::unique_ptr<vk::CommandPoolManager> cmdPoolManager;
		std::unique_ptr<vk::DescriptorManager> descriptorManager;
		std::unique_ptr<render::Renderer> renderer;
		std::unique_ptr<GameInstance> gameInstance;

	};
}