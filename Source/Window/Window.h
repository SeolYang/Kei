#pragma once
#include <PCH.h>

namespace sy::window
{
	class WindowBuilder;
	class Window
	{
	public:
		explicit Window(const WindowBuilder& builder);
		~Window() = default;

		Window(const window::Window&) = delete;
		Window(window::Window&&) = default;

		window::Window& operator=(const window::Window&) = delete;
		window::Window& operator=(window::Window&&) = default;

		[[nodiscard]] std::string_view GetTitle() const
		{
			return title;
		}

		[[nodiscard]] SDL_Window& GetSDLWindow() const
		{
			return *window;
		}

		[[nodiscard]] auto GetExtent() const
		{
			return extent;
		}

		void Startup();
		void Shutdown();

	private:
		std::string title;
		Extent2D<uint32_t> extent;
		SDL_Window* window;
	};
} // namespace sy::window
