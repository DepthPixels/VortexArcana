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

// Rendering helpers.
void Entity::RenderAlbedo(glm::mat4 viewMatrix) {
	for (SpriteRenderer2D* component : GetComponents<SpriteRenderer2D>()) {
		component->Render(viewMatrix);
	}
}

void Entity::RenderLights(std::vector<float>& lightData, int& lightIndex) {
	for (PointLight* component : GetComponents<PointLight>()) {
		component->RenderLights(lightData, lightIndex);
	}
}

void Entity::RenderOcclusion(glm::mat4 viewMatrix) {
	for (SpriteRenderer2D* component : GetComponents<SpriteRenderer2D>()) {
		component->RenderOcclusion(viewMatrix);
	}
}

// Children helpers.
void Entity::AddChild(Entity* child) {
	if (child == nullptr) return;
	if (std::find(this->children.begin(), this->children.end(), child) != this->children.end()) return; // Already a child.
	this->children.push_back(child);
}