#pragma once
#include "DirectXMath/DirectXMath.h"
using namespace DirectX;

// Ray
namespace Fuko
{
	struct Vector2;
	struct Vector4;
	struct Matrix;
	struct Quaternion;
	struct Plane;

	class Ray
	{
	public:
		Vector3 position;
		Vector3 direction;

		Ray() noexcept : position(0, 0, 0), direction(0, 0, 1) {}
		Ray(const Vector3& pos, const Vector3& dir) noexcept : position(pos), direction(dir) {}

		Ray(const Ray&) = default;
		Ray& operator=(const Ray&) = default;

		Ray(Ray&&) = default;
		Ray& operator=(Ray&&) = default;

		// Comparison operators
		bool operator == (const Ray& r) const noexcept;
		bool operator != (const Ray& r) const noexcept;

		// Ray operations
		bool Intersects(const BoundingSphere& sphere, _Out_ float& Dist) const noexcept;
		bool Intersects(const BoundingBox& box, _Out_ float& Dist) const noexcept;
		bool Intersects(const Vector3& tri0, const Vector3& tri1, const Vector3& tri2, _Out_ float& Dist) const noexcept;
		bool Intersects(const Plane& plane, _Out_ float& Dist) const noexcept;
	};
}
