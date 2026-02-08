#pragma once

#include "Core/Entity.h"
#include "Core/Components/SpriteRenderer2D.h"
#include "Core/Utility/Textures.h"

namespace Vortex {
	class Tile : public Entity {
	public:
		Tile(Texture2D spriteTexture, Vec2 position, const char* textureAddress) {
			bounds.w = 32.0f;
			bounds.h = 32.0f;
			bounds.position = position;
			Shader tileShader("assets/shaders/basicVertex.glsl", "assets/shaders/basicFragment.glsl");
			SpriteRenderer2D* spriteComponent = new SpriteRenderer2D(tileShader);
			spriteComponent->LoadSprite(textureAddress, true);
			AddComponent(spriteComponent);
		}
	};
}