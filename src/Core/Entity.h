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

		// Children Stuff.
		Entity* parent = nullptr;
		std::vector<Entity*> children;

		// Entity ID (this pointer)
		int* entityID;

		// Constructor
		Entity() {
			entityID = (int*)this;
		}

		// Property helpers.
		void SetPosition(float x, float y) {
			for (Entity* child : children) {
				Vortex::Vec2 delta = { child->bounds.position.x - bounds.position.x, child->bounds.position.y - bounds.position.y };
				child->SetPosition(x + delta.x, y + delta.y);
			}
			bounds.position.x = x;
			bounds.position.y = y;
		}

		void SetPosition(Vec2 position) {
			SetPosition(position.x, position.y);
		}

		void SetParent(Entity* parent) {
			this->parent = parent;
			parent->AddChild(this);
		}

		void RemoveChild(Entity* child) {
			if (child == nullptr) return;
			auto it = std::find(this->children.begin(), this->children.end(), child);
			if (it != this->children.end()) {
				this->children.erase(it);
			}
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

		// Rendering helpers.
		void RenderAlbedo(glm::mat4 viewMatrix);
		void RenderLights(std::vector<float>& lightData, int& lightIndex);
		void RenderOcclusion(glm::mat4 viewMatrix);

		// Children helpers.
		void AddChild(Entity* child);

		// Getters
		Entity* GetParent() {
			return parent;
		}

		Vec2 GetPosition() {
			return bounds.position;
		}

		Vec2 GetBounds() {
			return Vec2(this->bounds.w, this->bounds.h);
		}
		
		int*& EntityID() { 
			return entityID;
		}

		~Entity();
	};
}