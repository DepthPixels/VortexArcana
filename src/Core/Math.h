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
	};

	class Rect {
	public:
		float x, y, w, h;

		bool isPosInRect(Vec2 pos) {
			std::cout << std::endl << "Mouse Pos X: " << pos.x << " Y: " << pos.y << std::endl;
			std::cout << "Viewport Pos X: " << x << " Y: " << y << std::endl;
			std::cout << "Width: " << w << " Height: " << h << std::endl;
			return (pos.x > x &&
				pos.x < x + w &&
				pos.y > y &&
				pos.y < y + h);
		}
	};
}