#pragma once

#include "Core/Component.h"
#include "Core/Entity.h"
#include "Core/Utility/Math.h"

#include <functional>

namespace Vortex {
	class Physics2D : public Component {
	public:
		Physics2D();
		~Physics2D();

		void ApplyForce(Vortex::Vec2 force);

		void Update(float deltaTime) override {
			(defaultPhysics) ? Integrate(deltaTime) : customPhysicsFunction(this, deltaTime);
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

		bool IsUsingDefaultPhysics() {
			return defaultPhysics;
		}

		// Setters
		void SetCustomPhysicsFunction(std::function<void(Physics2D*, float)> func) {
			customPhysicsFunction = func;
			(customPhysicsFunction == nullptr) ? defaultPhysics = true : defaultPhysics = false;
		}

		/// <summary>
		/// Set whether to use the default physics integration or a custom function. If no custom function has been set, this will return -1 and not change the default physics state.
		/// </summary>
		/// <param name="useDefault"></param>
		/// <returns> 0 on success and -1 on fail.</returns>
		int setDefaultPhysics(bool useDefault) {
			if (useDefault && customPhysicsFunction == nullptr) {
				return -1;
			}
			defaultPhysics = useDefault;
			return 0;
		}

	private:
		Vec2 velocity = { 0.0f, 0.0f };
		Vec2 acceleration = { 0.0f, 0.0f };
		Vec2 forceAccumulator = { 0.0f, 0.0f };

		void Integrate(float deltaTime);

		float mass = 1.0f;

		bool defaultPhysics = true;
		std::function <void(Physics2D*, float)> customPhysicsFunction = nullptr;
	};
}