#include "Rigidbody.h"
#include <algorithm>

using namespace Vortex;

void Rigidbody::Initialize(CollisionShape shape) {
	switch (shape) {
		case CollisionShape::Rectangle:
			collider = new RectangleCollisionContainer(owner, owner->GetBounds());
			break;
		case CollisionShape::Circle:
			collider = new CircleCollisionContainer(owner, 50.0f);
			break;
        case CollisionShape::StaticOcclusion:
            collider = new StaticOcclusionCollisionContainer(owner, owner->GetBounds());
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
    // Circle vs Occlusion
    else if (thisShape == CollisionShape::Circle && otherShape == CollisionShape::StaticOcclusion) {
        return checkCircleOcclusion(other);
    }
    // Occlusion vs Circle
    else if (thisShape == CollisionShape::StaticOcclusion && otherShape == CollisionShape::Circle) {
        return checkOcclusionCircle(other);
    }
    // Rectangle vs Occlusion
    else if (thisShape == CollisionShape::Rectangle && otherShape == CollisionShape::StaticOcclusion) {
        return checkRectangleOcclusion(other);
    }
    // Occlusion vs Rectangle
    else if (thisShape == CollisionShape::StaticOcclusion && otherShape == CollisionShape::Rectangle) {
        return checkOcclusionRectangle(other);
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
        manifold.normal = (closestPoint - aCenter).normalized();
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
        manifold.normal = (aCenter - closestPoint).normalized();
    }

    return manifold;
}

Vortex::CollisionManifold Rigidbody::checkCircleOcclusion(Rigidbody* other) {
    Vortex::CollisionManifold manifold;
    CircleCollisionContainer* circ = reinterpret_cast<CircleCollisionContainer*>(collider);
    StaticOcclusionCollisionContainer* occ = reinterpret_cast<StaticOcclusionCollisionContainer*>(other->collider);

    // If the buffer hasn't been loaded from the GPU yet, abort to prevent crash
    if (occ->pixelBuffer.empty()) return manifold;

    Vec2 circCenter = owner->GetPosition() + circ->originOffset;
    Vec2 occPos = other->owner->GetPosition();

    // Transform circle center into the occlusion mask's local space
    Vec2 localPos = circCenter - occPos;

    int width = (int)occ->bounds.x;
    int height = (int)occ->bounds.y;

    // AABB check to check only pixel within circle's radius
    int minX = std::max(0, (int)(localPos.x - circ->radius));
    int maxX = std::min(width, (int)(localPos.x + circ->radius));
    int minY = std::max(0, (int)(localPos.y - circ->radius));
    int maxY = std::min(height, (int)(localPos.y + circ->radius));

    Vec2 totalNormal(0.0f, 0.0f);
    float maxPenetration = 0.0f;
    int hitCount = 0;

    for (int y = minY; y < maxY; ++y) {
        for (int x = minX; x < maxX; ++x) {
            // Flip back from OpenGL.
            int flippedY = (height - 1) - y;

            if (occ->pixelBuffer[flippedY * width + x] > 0) {
                // Vector pointing FROM Circle TO Pixel
                float dx = x - localPos.x;
                float dy = y - localPos.y;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist < circ->radius) {
                    manifold.isColliding = true;
                    hitCount++;

                    float pen = circ->radius - dist;
                    if (pen > maxPenetration) maxPenetration = pen;

                    if (dist > 0.0001f) {
                        totalNormal.x += (dx / dist);
                        totalNormal.y += (dy / dist);
                    }
                }
            }
        }
    }

    if (manifold.isColliding && hitCount > 0) {
        manifold.penetration = maxPenetration;
        manifold.normal = totalNormal.normalized();
    }

    return manifold;
}

Vortex::CollisionManifold Rigidbody::checkOcclusionCircle(Rigidbody* other) {
    // Flip normal vector.
    Vortex::CollisionManifold manifold = other->checkCircleOcclusion(this);
    if (manifold.isColliding) {
        manifold.normal = Vec2(manifold.normal.x * -1.0f, manifold.normal.y * -1.0f);
    }
    return manifold;
}

Vortex::CollisionManifold Rigidbody::checkRectangleOcclusion(Rigidbody* other) {
    Vortex::CollisionManifold manifold;
    RectangleCollisionContainer* rect = reinterpret_cast<RectangleCollisionContainer*>(collider);
    StaticOcclusionCollisionContainer* occ = reinterpret_cast<StaticOcclusionCollisionContainer*>(other->collider);

    // If the buffer hasn't been loaded from the GPU yet, abort to prevent crash
    if (occ->pixelBuffer.empty()) return manifold;

    // Rect AABB check.
    Vec2 rectHalfExtents = rect->bounds / 2.0f;
    Vec2 rectCenter = owner->GetPosition() + rect->originOffset + rectHalfExtents;
    Vec2 occPos = other->owner->GetPosition();

    Vec2 localCenter = rectCenter - occPos;

    int width = (int)occ->bounds.x;
    int height = (int)occ->bounds.y;

    int minX = std::max(0, (int)(localCenter.x - rectHalfExtents.x));
    int maxX = std::min(width, (int)(localCenter.x + rectHalfExtents.x));
    int minY = std::max(0, (int)(localCenter.y - rectHalfExtents.y));
    int maxY = std::min(height, (int)(localCenter.y + rectHalfExtents.y));

    Vec2 totalNormal(0.0f, 0.0f);
    float maxPenetration = 0.0f;
    int hitCount = 0;

    for (int y = minY; y < maxY; ++y) {
        for (int x = minX; x < maxX; ++x) {
            // Flip back from OpenGL.
            int flippedY = (height - 1) - y;

            if (occ->pixelBuffer[flippedY * width + x] > 0) {
                manifold.isColliding = true;
                hitCount++;

                // Find the distance from this pixel to all 4 edges of the rectangle
                float distLeft = x - (localCenter.x - rectHalfExtents.x);
                float distRight = (localCenter.x + rectHalfExtents.x) - x;
                float distTop = y - (localCenter.y - rectHalfExtents.y);
                float distBottom = (localCenter.y + rectHalfExtents.y) - y;

                // The smallest distance shows which edge the pixel penetrated
                float minPen = std::min({ distLeft, distRight, distTop, distBottom });

                if (minPen > maxPenetration) {
                    maxPenetration = minPen;
                }

                // Add to the average normal pointing FROM the Rectangle TO the pixel
                if (minPen == distLeft) totalNormal.x -= 1.0f;
                else if (minPen == distRight) totalNormal.x += 1.0f;
                else if (minPen == distTop) totalNormal.y -= 1.0f;
                else if (minPen == distBottom) totalNormal.y += 1.0f;
            }
        }
    }

    if (manifold.isColliding && hitCount > 0) {
        manifold.penetration = maxPenetration;
        manifold.normal = totalNormal.normalized();
    }

    return manifold;
}

Vortex::CollisionManifold Rigidbody::checkOcclusionRectangle(Rigidbody* other) {
    // Flip normal
    Vortex::CollisionManifold manifold = other->checkRectangleOcclusion(this);
    if (manifold.isColliding) {
        manifold.normal = Vec2(manifold.normal.x * -1.0f, manifold.normal.y * -1.0f);
    }
    return manifold;
}