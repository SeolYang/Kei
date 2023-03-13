#pragma once
#include <PCH.h>

namespace sy::component
{
	struct HierarchyComponent : ecs::Component
	{
		ecs::Entity Parent = ecs::INVALID_ENTITY_HANDLE;
	};

	inline bool HasParent(const HierarchyComponent& component)
	{
		return component.Parent != ecs::INVALID_ENTITY_HANDLE;
	}
}
