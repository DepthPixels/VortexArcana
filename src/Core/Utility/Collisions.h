#pragma once

#include "Core/Utility/Math.h"
#include "Core/Utility/Textures.h"
#include "Core/Components/SpriteRenderer2D.h"

namespace Vortex {
	class Entity;

	enum class CollisionShape {
		Rectangle,
		Circle,
		StaticOcclusion
	};

	class CollisionContainer {
	public:
		Vortex::Vec2 originOffset = { 0.0f, 0.0f };
		Entity* owner = nullptr;

		virtual ~CollisionContainer() {}
		virtual CollisionShape GetCollisionShapeType() = 0;
	};

	class CollisionManifold {
	public:
		bool isColliding = false;
		Vortex::Vec2 normal = { 0.0f, 0.0f };
		float penetration = 0.0f;
	};

	class RectangleCollisionContainer : public CollisionContainer {
	public:
		Vortex::Vec2 bounds;

		RectangleCollisionContainer(Entity* owner_ptr, Vortex::Vec2 init_bounds) {
			owner = owner_ptr;
			bounds = init_bounds;
		}

		CollisionShape GetCollisionShapeType() override {
			return CollisionShape::Rectangle;
		}
	};

	class CircleCollisionContainer : public CollisionContainer {
	public:
		float radius;

		CircleCollisionContainer(Entity* owner_ptr, float init_radius) {
			owner = owner_ptr;
			radius = init_radius;
		}

		CollisionShape GetCollisionShapeType() override {
			return CollisionShape::Circle;
		}
	};

	class StaticOcclusionCollisionContainer : public CollisionContainer {
	public:
		Vortex::Vec2 bounds;
		Texture2D occlusionTexture;
		std::vector<unsigned char> pixelBuffer;

		StaticOcclusionCollisionContainer(Entity* owner_ptr, Vortex::Vec2 init_bounds) {
			owner = owner_ptr;
			bounds = init_bounds;

			occlusionTexture.internalFormat = GL_R8;
			occlusionTexture.imageFormat = GL_RED;
			occlusionTexture.Generate(bounds.x, bounds.y, nullptr);
		}

		void BakeCollisionMask() {
			SpriteRenderer2D* spriteRenderer = owner->GetComponent<SpriteRenderer2D>();
			if (spriteRenderer) {
				spriteRenderer->BakeOcclusion(this);
			}
		}

		void LoadBufferFromTexture() {
			pixelBuffer.resize(bounds.x * bounds.y);
			glBindTexture(GL_TEXTURE_2D, occlusionTexture.ID);
			// Download the texture data back to CPU
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, pixelBuffer.data());
		}

		CollisionShape GetCollisionShapeType() override {
			return CollisionShape::StaticOcclusion;
		}
	};
}