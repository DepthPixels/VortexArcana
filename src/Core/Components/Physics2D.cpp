#include "Physics2D.h"

using namespace Vortex;
Physics2D::Physics2D()
	: mass(1.0f), velocity({ 0.0f, 0.0f }), acceleration({ 0.0f, 0.0f }), forceAccumulator({ 0.0f, 0.0f }) {
}

Physics2D::~Physics2D() {}


void Physics2D::ApplyForce(Vortex::Vec2 force) {
	if (this->owner->isBeingDragged) return;

	forceAccumulator += force;
}


void Physics2D::Integrate(float deltaTime) {
	if (this->owner->isBeingDragged) {
		acceleration = { 0.0f, 0.0f };
		velocity = { 0.0f, 0.0f };
		forceAccumulator = { 0.0f, 0.0f };
		return;
	}

	acceleration = forceAccumulator * (1.0f / mass);
	velocity += acceleration * deltaTime;
	this->owner->bounds.position += velocity * deltaTime;
	forceAccumulator = { 0.0f, 0.0f };
}