#pragma once

#include "Core/Component.h"
#include "Core/Utility/Math.h"
#include <vector>
#include <string>
#include <SDL3/SDL.h>

namespace Vortex {
	class Entity {
	public:
		Vortex::Vec2 velocity;
		Vortex::Rect bounds = {0.0f, 0.0f, 128.0f, 128.0f};
		float rotation = 0.0f;
		bool isSelected = false;
		bool isBeingDragged = false;
		std::string name = "Default";

		// Components.
		std::vector<Component*> components;

		// Component helpers.
		void AddComponent(Component* component);
		void UpdateComponents(float deltaTime);
		void RenderComponents();

		~Entity();

		/* To be moved when Physics2D is created.
		void ApplyForce(Vortex::Vec2 force) {
			if (isBeingDragged) return;

			forceAccumulator += force;
		}

		
		void Integrate(float deltaTime) {
			if (isBeingDragged) return;

			acceleration = forceAccumulator * (1.0f / mass);
			velocity += acceleration * deltaTime;
			bounds.position += velocity * deltaTime;
			forceAccumulator = { 0.0f, 0.0f };
		}
		*/
	};
}