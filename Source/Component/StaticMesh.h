#pragma once
#include <PCH.h>

namespace sy::render
{
	class Mesh;
}

namespace sy::vk
{
	class Texture;
}

namespace sy::component
{
	struct StaticMesh : ecs::Component
	{
		Handle<render::Mesh> Mesh;
		Handle<vk::Texture> Texture;
	};
}
