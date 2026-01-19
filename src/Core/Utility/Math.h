#pragma once
#include <iostream>
#include "imgui.h"

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
		explicit operator glm::vec2() const {
			return glm::vec2{ x, y };
		}
	};

	struct Vec3 {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		// Scalar Operations
		Vec3 operator*(float scalar) const {
			return Vec3{
				x * scalar,
				y * scalar,
				z * scalar
			};
		}
		void operator*=(float scalar) {
			x *= scalar;
			y *= scalar;
			z *= scalar;
		}
		// Vector Operations
		Vec3 operator+(const Vec3& other) const {
			return Vec3{
				x + other.x,
				y + other.y,
				z + other.z
			};
		}
		void operator+=(const Vec3& other) {
			x += other.x;
			y += other.y;
			z += other.z;
		}
		Vec3 operator*(const Vec3& other) const {
			return Vec3{
				x * other.x,
				y * other.y,
				z * other.z
			};
		}
		void operator*=(const Vec3& other) {
			x *= other.x;
			y *= other.y;
			z *= other.z;
		}

		// Type Conversions
		explicit operator glm::vec3() const {
			return glm::vec3{ x, y, z };
		}

	};
	
	struct Vec4 {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float w = 0.0f;
		// Scalar Operations
		Vec4 operator*(float scalar) const {
			return Vec4{
				x * scalar,
				y * scalar,
				z * scalar,
				w * scalar
			};
		}
		void operator*=(float scalar) {
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
		}
		// Vector Operations
		Vec4 operator+(const Vec4& other) const {
			return Vec4{
				x + other.x,
				y + other.y,
				z + other.z,
				w + other.w
			};
		}
		void operator+=(const Vec4& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
		}
		Vec4 operator*(const Vec4& other) const {
			return Vec4{
				x * other.x,
				y * other.y,
				z * other.z,
				w * other.w
			};
		}
		void operator*=(const Vec4& other) {
			x *= other.x;
			y *= other.y;
			z *= other.z;
			w *= other.w;
		}

		// Type Conversions
		explicit operator glm::vec4() const {
			return glm::vec4{ x, y, z, w };
		}
	};

	class Rect {
	public:
		Vec2 position;
		float w = 0.0f;
		float h = 0.0f;

		bool isPosInRect(Vec2 pos) {
			return (pos.x > position.x &&
				pos.x < position.x + w &&
				pos.y > position.y &&
				pos.y < position.y + h);
		}
	};
}