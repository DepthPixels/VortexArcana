#pragma once

#include "Core/Component.h"
#include "Core/Utility/Math.h"
#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <SDL3/SDL.h>

namespace Vortex {
	class Entity {
	public:
		Vortex::Rect bounds = {0.0f, 0.0f, 128.0f, 128.0f};
		float rotation = 0.0f;
		bool isSelected = false;
		bool isBeingDragged = false;
		std::string name = "Default";
		std::set<std::string> scriptsAttached;

		// Components.
		std::vector<Component*> components;
		// Entity ID (this pointer)
		int* entityID;

		// Constructor
		Entity() {
			entityID = (int*)this;
		}

		// Component helpers.
		void AddComponent(Component* component);

		std::vector<Component*> GetAllComponents() {
			return components;
		}

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
			std::vector<T*> foundComponents;
			for (Component* component : components) {
				T* casted = dynamic_cast<T*>(component);
				if (casted != nullptr) {
					foundComponents.push_back(casted);
				}
			}
			return foundComponents;
		}

		void UpdateComponents(float deltaTime);
		void RenderComponents();

		// Getters
		int*& EntityID() { 
			return entityID;
		}

		~Entity();
	};
}