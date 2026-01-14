#pragma once
#include <iostream>

namespace Vortex {
	struct Vec2 {
		float x = 0.0f;
		float y = 0.0f;

		// Scalar Operations
		Vec2 operator*(float scalar) const {
			return Vec2{
				x * scalar,
				y * scalar
			};
		}
		void operator*=(float scalar) {
			x *= scalar;
			y *= scalar;
		}

		// Vector Operations
		Vec2 operator+(const Vec2& other) const {
			return Vec2{
				x + other.x,
				y + other.y
			};
		}
		void operator+=(const Vec2& other) {
			x += other.x;
			y += other.y;
		}
		Vec2 operator*(const Vec2& other) const {
			return Vec2{
				x * other.x,
				y * other.y
			};
		}
		void operator*=(const Vec2& other) {
			x *= other.x;
			y *= other.y;
		}

		// Type Conversions
		explicit operator ImVec2() const {
			return ImVec2{ x, y };
		}
	};

	class Rect {
	public:
		Vec2 position;
		float w, h;

		bool isPosInRect(Vec2 pos) {
			return (pos.x > position.x &&
				pos.x < position.x + w &&
				pos.y > position.y &&
				pos.y < position.y + h);
		}
	};
}