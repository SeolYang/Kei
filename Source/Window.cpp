#include <Core.h>
#include <Window.h>

namespace sy
{
	Window::Window(std::string_view title, Extent2D<uint32_t> extent) :
		title(title),
		extent(extent)
	{
		Startup();
	}

	Window::~Window()
	{
		Cleanup();
	}

	void Window::Startup()
	{
		SY_ASSERT(extent.IsValid(), "Extent may contains non-valid values.");

		SDL_Init(SDL_INIT_VIDEO);

		constexpr SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN;
		window = SDL_CreateWindow(
			"Vulkan Engine",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			extent.width,
			extent.height,
			windowFlags
		);

		SY_ASSERT(window != nullptr, "Failed to create sdl window");
	}

	void Window::Cleanup()
	{
		if (window != nullptr)
		{
			SDL_DestroyWindow(window);
		}
	}
}