#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Core/Component.h"
#include "Core/Entity.h"
#include "Core/Utility/Shader.h"

namespace Vortex {
	class PointLight : public Component {
	public:
		Vec3 color = { 1.0f, 1.0f, 1.0f };
		float brightness = 1.0f;
		float falloff = 2.0f;
		float radius = 200.0f;
		bool active = true;

		PointLight(Shader* shader = nullptr);
		~PointLight();

		void Illuminate(Vortex::Vec2 position, glm::mat4 viewMatrix);

		void Render(glm::mat4 viewMatrix) override {
			if (active) {
				Illuminate(owner->bounds.position, viewMatrix);
			}
		}

		// Getters
		std::string GetName() override {
			return "PointLight";
		}

	private:
		Shader* shader;
		unsigned int quadVAO;

		void initRenderData();
	};
}