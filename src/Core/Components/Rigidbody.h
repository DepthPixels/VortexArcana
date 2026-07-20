#pragma once

#include "Core/Entity.h"
#include "Core/Component.h"
#include "Core/Utility/Math.h"
#include "Core/Utility/Collisions.h"

namespace Vortex {
	class Rigidbody : public Component {
	public:
		CollisionContainer* collider;

		void Initialize(CollisionShape shape);

		Rigidbody() {};
		~Rigidbody() {}

		Vortex::CollisionManifold checkCollision(Rigidbody* other);

		// Checkers
		Vortex::CollisionManifold checkRectangleRectangle(Rigidbody* other);
		Vortex::CollisionManifold checkCircleCircle(Rigidbody* other);
		Vortex::CollisionManifold checkRectangleCircle(Rigidbody* other);
		Vortex::CollisionManifold checkCircleRectangle(Rigidbody* other);
		Vortex::CollisionManifold checkCircleOcclusion(Rigidbody* other);
		Vortex::CollisionManifold checkOcclusionCircle(Rigidbody* other);
		Vortex::CollisionManifold checkRectangleOcclusion(Rigidbody* other);
		Vortex::CollisionManifold checkOcclusionRectangle(Rigidbody* other);
	};
}