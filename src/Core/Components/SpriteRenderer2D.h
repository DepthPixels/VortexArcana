#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include "Core/Component.h"
#include "Core/Entity.h"
#include "Core/Utility/Shader.h"
#include "Core/Utility/Textures.h"

namespace Vortex {
	class StaticOcclusionCollisionContainer;

	class SpriteRenderer2D : public Component {
	public:
		SpriteRenderer2D(Shader* shader = nullptr);
		~SpriteRenderer2D();

		bool spriteAssigned = false;
		std::string spriteLocation = "None";

		Vortex::Texture2D texture;

		void LoadSprite(const char* location, bool alpha);

		void DrawSprite(Vortex::Vec2 position, Vortex::Vec2 size, float rotation, Vortex::Vec3 color, glm::mat4 viewMatrix);
		void DrawOcclusion(Vortex::Vec2 position, Vortex::Vec2 size, float rotation, glm::mat4 viewMatrix);
		void BakeOcclusion(StaticOcclusionCollisionContainer* container);

		void Render(glm::mat4 viewMatrix) override {
			if (spriteAssigned) {
				DrawSprite(owner->bounds.position, Vortex::Vec2(owner->bounds.w, owner->bounds.h), owner->rotation, Vortex::Vec3(1.0f, 1.0f, 1.0f), viewMatrix);
			}
		}

		void RenderOcclusion(glm::mat4 viewMatrix) {
			if (spriteAssigned) {
				DrawOcclusion(owner->bounds.position, Vortex::Vec2(owner->bounds.w, owner->bounds.h), owner->rotation, viewMatrix);
			}
		}

		// Getters
		std::string GetName() override {
			return "SpriteRenderer2D";
		}

	private:
		Shader* shader;
		Shader* singleColorShader;
		Shader* occlusionShader;
		unsigned int quadVAO;

		void initRenderData();
	};
}