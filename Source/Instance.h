#pragma once
#include <Core.h>

namespace sy
{
	class Window;
	class VulkanInstance;
	class Renderer;
	class GameInstance;
	class Instance
	{
	public:
		Instance();
		Instance(const Instance&) = delete;
		Instance(Instance&&) = default;

		~Instance();

		Instance& operator=(const Instance&) = delete;
		Instance& operator=(Instance&&) = default;

		void Run();

	private:
		void Startup();
		void Cleanup();

	private:
		std::unique_ptr<Timer> timer;
		std::unique_ptr<Window> window;
		std::unique_ptr<VulkanInstance> vulkanInstance;
		std::unique_ptr<Renderer> renderer;
		std::unique_ptr<GameInstance> gameInstance;

	};
}