#pragma once
#include <PCH.h>

namespace sy::window
{
	class Window;
	class WindowBuilder
	{
	public:
		WindowBuilder()
		{
		}

		WindowBuilder& SetTitle(const std::string_view title)
		{
			this->title = title;
			return *this;
		}

		WindowBuilder& SetExtent(const Extent2D<uint32_t> extent)
		{
			this->extent = extent;
			return *this;
		}

		// WindowBuilder& SetFullscreenMode()

		std::unique_ptr<Window> Build() const;

	private:
		friend Window;
		std::string title = "Kei";
		Extent2D<uint32_t> extent = { 1280, 720 };
	};
} // namespace sy::window