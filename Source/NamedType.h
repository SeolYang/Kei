#pragma once
#include <Core.h>

namespace sy
{
	struct NamedType
	{
	public:
		NamedType(const std::string_view name) :
			name(name)
		{
		}

		virtual ~NamedType() = default;

		void SetName(const std::string_view name)
		{
			this->name = name;
		}

		[[nodiscard]] std::string_view GetName() const { return name; }
		[[nodiscard]] std::string& GetName() { return name; }

	private:
		std::string name;
	};
}