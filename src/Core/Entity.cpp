#include "Core/Entity.h"

using namespace Vortex;

Entity::~Entity() {
	for (Component* component : components) {
		delete component;
	}
	components.clear();
}

// Component helpers.
void Entity::AddComponent(Component* component) {
	component->owner = this;
	components.push_back(component);
	component->Init();
}

void Entity::UpdateComponents(float deltaTime) {
	for (Component* component : components) {
		component->Update(deltaTime);
	}
}

void Entity::RenderComponents(glm::mat4 viewMatrix) {
	for (Component* component : components) {
		component->Render(viewMatrix);
	}
}