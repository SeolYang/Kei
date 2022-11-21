#pragma once
#include <Core.h>

namespace sy
{
	class Window
	{
	public:
		Window(std::string_view title, Extent2D<uint32_t> extent);
		~Window();

		Window(const Window&) = delete;
		Window(Window&&) = default;

		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = default;

		[[nodiscard]] std::string_view GetWindowTitle() const { return title; }
		[[nodiscard]] SDL_Window& GetSDLWindow() const { return *window; }

	private:
		void Startup();
		void Cleanup();

	private:
		std::string title;
		Extent2D<uint32_t> extent;
		SDL_Window* window;

	};
}