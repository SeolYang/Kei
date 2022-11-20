#pragma once
#include <Core.h>

namespace sy
{
	class Window;
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

		void Startup();
		void Run();

	private:
		std::unique_ptr<Timer> timer;
		std::unique_ptr<Window> window;
		std::unique_ptr<GameInstance> gameInstance;

	};
}