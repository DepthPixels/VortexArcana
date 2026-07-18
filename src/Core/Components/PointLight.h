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
		float brightness = 0.9f;
		float falloff = 2.0f;
		float radius = 400.0f;
		bool active = true;

		~PointLight();

		void Illuminate(Vortex::Vec2 position, std::vector<float>& lightData, int& lightIndex);

		void RenderLights(std::vector<float>& lightData, int& lightIndex) {
			if (active) {
				Illuminate(owner->bounds.position, lightData, lightIndex);
			}
		}

		// Getters
		std::string GetName() override {
			return "PointLight";
		}

	private:
	};
}