#include "Rigidbody.h"
#include <algorithm>

using namespace Vortex;

void Rigidbody::Initialize(CollisionShape shape) {
	switch (shape) {
		case CollisionShape::Rectangle:
			std::cout << "Owner is " << owner << std::endl;
			collider = new RectangleCollisionContainer(owner, owner->GetBounds());
			break;
		case CollisionShape::Circle:
			collider = new CircleCollisionContainer(owner, 50.0f);
			break;
		default:
			// Redundant
			collider = new RectangleCollisionContainer(owner, owner->GetBounds());
			break;
	}
}

Vortex::CollisionManifold Rigidbody::checkCollision(Rigidbody* other) {
	CollisionShape otherShape = other->collider->GetCollisionShapeType();
	CollisionShape thisShape = collider->GetCollisionShapeType();

    // Rectangle vs Rectangle
    if (thisShape == CollisionShape::Rectangle && otherShape == CollisionShape::Rectangle) {
        return checkRectangleRectangle(other);
    }
    // Circle vs Circle
    else if (thisShape == CollisionShape::Circle && otherShape == CollisionShape::Circle) {
        return checkCircleCircle(other);
    }
    // Circle vs Rectangle
    else if (thisShape == CollisionShape::Circle && otherShape == CollisionShape::Rectangle) {
        return checkCircleRectangle(other);
    }
    // Rectangle vs Circle
    else if (thisShape == CollisionShape::Rectangle && otherShape == CollisionShape::Circle) {
        return checkRectangleCircle(other);
    }

    return CollisionManifold();
}

Vortex::CollisionManifold Rigidbody::checkRectangleRectangle(Rigidbody* other) {
    Vortex::CollisionManifold manifold;

    RectangleCollisionContainer* colA = reinterpret_cast<RectangleCollisionContainer*>(collider);
    RectangleCollisionContainer* colB = reinterpret_cast<RectangleCollisionContainer*>(other->collider);

    // Calculate Half-Extents (Half the width and height)
    Vec2 aHalfExtents = colA->bounds / 2.0f;
    Vec2 bHalfExtents = colB->bounds / 2.0f;

    // Calculate Centers
    Vec2 aCenter = owner->GetPosition() + colA->originOffset + aHalfExtents;
    Vec2 bCenter = other->owner->GetPosition() + colB->originOffset + bHalfExtents;

    // Calculate distance between centers
    Vec2 distance = bCenter - aCenter;

    // Calculate absolute overlap on both axes
    float xOverlap = (aHalfExtents.x + bHalfExtents.x) - std::abs(distance.x);
    float yOverlap = (aHalfExtents.y + bHalfExtents.y) - std::abs(distance.y);

    // If both overlaps are greater than 0, a collision is occurring
    if (xOverlap > 0 && yOverlap > 0) {
        manifold.isColliding = true;

        // Find the axis of least penetration
        if (xOverlap < yOverlap) {
            // Collision is on the X axis
            manifold.penetration = xOverlap;
            // Determine direction (Left or Right)
            if (distance.x < 0) {
                manifold.normal = Vec2(-1.0f, 0.0f);
            }
            else {
                manifold.normal = Vec2(1.0f, 0.0f);
            }
        }
        else {
            // Collision is on the Y axis
            manifold.penetration = yOverlap;
            // Determine direction (Up or Down)
            if (distance.y < 0) {
                manifold.normal = Vec2(0.0f, -1.0f);
            }
            else {
                manifold.normal = Vec2(0.0f, 1.0f);
            }
        }
    }

    return manifold;
}

Vortex::CollisionManifold Rigidbody::checkCircleCircle(Rigidbody* other) {
    Vortex::CollisionManifold manifold;

    CircleCollisionContainer* colA = reinterpret_cast<CircleCollisionContainer*>(collider);
    CircleCollisionContainer* colB = reinterpret_cast<CircleCollisionContainer*>(other->collider);

    // Calculate Centers
    Vec2 aCenter = owner->GetPosition() + colA->originOffset;
    Vec2 bCenter = other->owner->GetPosition() + colB->originOffset;

    float distance = sqrt(pow(bCenter.x - aCenter.x, 2) + pow(bCenter.y - aCenter.y, 2));

    if (distance < colA->radius + colB->radius) {
        std::cout << "CircleCircle Collision Success with aCenter (" << aCenter.x << ", " << aCenter.y << ") and bCenter (" << bCenter.x << ", " << bCenter.y << ")" << std::endl;
        manifold.isColliding = true;
        manifold.penetration = (colA->radius + colB->radius) - distance;
        manifold.normal = (bCenter - aCenter).normalized();
    }

    return manifold;
}

Vortex::CollisionManifold Rigidbody::checkCircleRectangle(Rigidbody* other) {
    Vortex::CollisionManifold manifold;

    CircleCollisionContainer* colA = reinterpret_cast<CircleCollisionContainer*>(collider);
    RectangleCollisionContainer* colB = reinterpret_cast<RectangleCollisionContainer*>(other->collider);

    // Calculate Half-Extents (Half the width and height)
    Vec2 bHalfExtents = colB->bounds / 2.0f;

    // Calculate Centers
    Vec2 aCenter = owner->GetPosition() + colA->originOffset;
    Vec2 bCenter = other->owner->GetPosition() + colB->originOffset + bHalfExtents;

    Vec2 diff = aCenter - bCenter;

    Vec2 clamped;
    clamped.x = std::clamp(diff.x, -bHalfExtents.x, bHalfExtents.x);
    clamped.y = std::clamp(diff.y, -bHalfExtents.y, bHalfExtents.y);

    Vec2 closestPoint = bCenter + clamped;

    float distance = sqrt(pow(closestPoint.x - aCenter.x, 2) + pow(closestPoint.y - aCenter.y, 2));

    if (distance < colA->radius) {
        manifold.isColliding = true;
        manifold.penetration = colA->radius - distance;
        manifold.normal = (aCenter - closestPoint).normalized();
    }

    return manifold;
}

Vortex::CollisionManifold Rigidbody::checkRectangleCircle(Rigidbody* other) {
    Vortex::CollisionManifold manifold;

    CircleCollisionContainer* colA = reinterpret_cast<CircleCollisionContainer*>(other->collider);
    RectangleCollisionContainer* colB = reinterpret_cast<RectangleCollisionContainer*>(collider);

    // Calculate Half-Extents (Half the width and height)
    Vec2 bHalfExtents = colB->bounds / 2.0f;

    // Calculate Centers
    Vec2 aCenter = other->owner->GetPosition() + colA->originOffset;
    Vec2 bCenter = owner->GetPosition() + colB->originOffset + bHalfExtents;

    Vec2 diff = aCenter - bCenter;

    Vec2 clamped;
    clamped.x = std::clamp(diff.x, -bHalfExtents.x, bHalfExtents.x);
    clamped.y = std::clamp(diff.y, -bHalfExtents.y, bHalfExtents.y);

    Vec2 closestPoint = bCenter + clamped;

    float distance = sqrt(pow(closestPoint.x - aCenter.x, 2) + pow(closestPoint.y - aCenter.y, 2));

    if (distance < colA->radius) {
        manifold.isColliding = true;
        manifold.penetration = colA->radius - distance;
        manifold.normal = (closestPoint - aCenter).normalized();
    }

    return manifold;
}