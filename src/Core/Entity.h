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

		// Component helpers.
		void AddComponent(Component* component);
		template <typename T>
		T* GetComponent() {
			for (Component* component : components) {
				T* casted = dynamic_cast<T*>(component);
				if (casted != nullptr) {
					return casted;
				}
			}
			return nullptr;
		}

		template <typename T>
		std::vector<T*> GetComponents() {
			for (Component* component : components) {
				std::vector<T*> foundComponents;
				T* casted = dynamic_cast<T*>(component);
				if (casted != nullptr) {
					foundComponents.push_back(casted);
				}
				return foundComponents;
			}
		}

		void UpdateComponents(float deltaTime);
		void RenderComponents();

		~Entity();
	private:
		// Components.
		std::vector<Component*> components;
	};
}