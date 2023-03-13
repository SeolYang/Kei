#pragma once

namespace sy
{
	template <typename T>
	struct Extent2D
	{
	public:
		[[nodiscard]] bool IsValid() const
		{
			return (width > 0) && (height > 0);
		}

	public:
		T width, height;
	};

	template <typename T>
	struct Extent3D
	{
	public:
		[[nodiscard]] bool IsValid() const
		{
			return (width > 0) && (height > 0) && (depth > 0);
		}

	public:
		T width, height, depth;
	};
}
