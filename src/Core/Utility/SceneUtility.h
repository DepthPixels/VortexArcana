#pragma once

#include "Core/Entity.h"
#include "Core/Components/SpriteRenderer2D.h"
#include "Core/Components/Physics2D.h"

#include <ryml.hpp>
#include <ryml_std.hpp>
#include <fstream>

namespace Vortex {
	class SceneUtility {
	public:
		static void SaveScene(std::vector<Entity*>& entities, const std::string& filePath);
		static void LoadScene(std::vector<Entity*>& entities, const std::string& filePath);
	};
}