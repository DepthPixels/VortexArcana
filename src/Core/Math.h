#pragma once

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
	};

	struct Rect {
		float x, y, w, h;
	};
}