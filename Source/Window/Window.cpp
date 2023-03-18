#include <PCH.h>
#include <Window/Window.h>

namespace sy::window
{
	Window::Window(const std::string_view title, const Extent2D<uint32_t> extent)
		: title(title), extent(extent), window(nullptr)
	{
	}

	void Window::Startup()
	{
		SY_ASSERT(extent.IsValid(), "Extent may contains non-valid values.");

		constexpr SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN;
		window = SDL_CreateWindow(
			title.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			extent.width,
			extent.height,
			windowFlags);

		SY_ASSERT(window != nullptr, "Failed to create sdl window");
	}

	void Window::Shutdown()
	{
		if (window != nullptr)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
		}
	}
} // namespace sy::window
