#pragma once

#include "Core/Component.h"
#include "Core/Entity.h"
#include "Core/Utility/Math.h"

namespace Vortex {
	class Physics2D : public Component {
	public:
		Physics2D();
		~Physics2D();

		void ApplyForce(Vortex::Vec2 force);

		void Update(float deltaTime) override {
			Integrate(deltaTime);
		}

		// Getters
		std::string GetName() override {
			return "Physics2D";
		}

		float& Mass() {
			return mass;
		}

		Vec2& Velocity() {
			return velocity;
		}

		Vec2& Acceleration() {
			return acceleration;
		}
	private:
		Vec2 velocity = { 0.0f, 0.0f };
		Vec2 acceleration = { 0.0f, 0.0f };
		Vec2 forceAccumulator = { 0.0f, 0.0f };

		void Integrate(float deltaTime);

		float mass = 1.0f;
	};
}