#pragma once
#include <PCH.h>

namespace sy::render
{
	class Model
	{
	public:
		//ecs::Entity ToEntity() const;

	private:
		std::vector<Handle<class Mesh>> meshes;
	};
}
