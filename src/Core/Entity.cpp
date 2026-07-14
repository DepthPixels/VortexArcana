#include "Core/Entity.h"
#include "Components/SpriteRenderer2D.h"
#include "Components/PointLight.h"

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

void Entity::RenderAlbedo(glm::mat4 viewMatrix) {
	for (Component* component : GetComponents<SpriteRenderer2D>()) {
		component->Render(viewMatrix);
	}
}

void Entity::RenderLights(glm::mat4 viewMatrix) {
	for (Component* component : GetComponents<PointLight>()) {
		component->Render(viewMatrix);
	}
}