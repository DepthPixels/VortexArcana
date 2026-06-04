#pragma once

#include "Core/Entity.h"
#include "Core/Components/SpriteRenderer2D.h"
#include "Core/Utility/Textures.h"

namespace Vortex {
	class Tile : public Entity {
	public:
		Tile(Vec2 position, std::string textureAddress) {
			bounds.w = 32.0f;
			bounds.h = 32.0f;
			bounds.position = position;
			Shader* tileShader = new Shader("assets/shaders/basicVertex.glsl", "assets/shaders/basicFragment.glsl");
			SpriteRenderer2D* spriteComponent = new SpriteRenderer2D(tileShader);
			spriteComponent->LoadSprite(textureAddress.c_str(), true);
			AddComponent(spriteComponent);
		}
	};
}