#pragma once
#include "DirectXMath/DirectXMath.h"
using namespace DirectX;

// Plane
namespace Fuko
{
	struct Vector2;
	struct Vector4;
	struct Matrix;
	struct Quaternion;
	struct Plane;

	struct Plane : public XMFLOAT4
	{
		Plane() noexcept : XMFLOAT4(0.f, 1.f, 0.f, 0.f) {}
		constexpr Plane(float ix, float iy, float iz, float iw) noexcept : XMFLOAT4(ix, iy, iz, iw) {}
		Plane(const Vector3& normal, float d) noexcept : XMFLOAT4(normal.x, normal.y, normal.z, d) {}
		Plane(const Vector3& point1, const Vector3& point2, const Vector3& point3) noexcept;
		Plane(const Vector3& point, const Vector3& normal) noexcept;
		explicit Plane(const Vector4& v) noexcept : XMFLOAT4(v.x, v.y, v.z, v.w) {}
		explicit Plane(_In_reads_(4) const float *pArray) noexcept : XMFLOAT4(pArray) {}
		Plane(FXMVECTOR V) noexcept { XMStoreFloat4(this, V); }
		Plane(const XMFLOAT4& p) noexcept { this->x = p.x; this->y = p.y; this->z = p.z; this->w = p.w; }
		explicit Plane(const XMVECTORF32& F) noexcept { this->x = F.f[0]; this->y = F.f[1]; this->z = F.f[2]; this->w = F.f[3]; }

		Plane(const Plane&) = default;
		Plane& operator=(const Plane&) = default;

		Plane(Plane&&) = default;
		Plane& operator=(Plane&&) = default;

		operator XMVECTOR() const noexcept { return XMLoadFloat4(this); }

		// Comparison operators
		bool operator == (const Plane& p) const noexcept;
		bool operator != (const Plane& p) const noexcept;

		// Assignment operators
		Plane& operator= (const XMVECTORF32& F) noexcept { x = F.f[0]; y = F.f[1]; z = F.f[2]; w = F.f[3]; return *this; }

		// Properties
		Vector3 Normal() const noexcept { return Vector3(x, y, z); }
		void Normal(const Vector3& normal) noexcept { x = normal.x; y = normal.y; z = normal.z; }

		float D() const noexcept { return w; }
		void D(float d) noexcept { w = d; }

		// Plane operations
		void Normalize() noexcept;
		void Normalize(Plane& result) const noexcept;

		float Dot(const Vector4& v) const noexcept;
		float DotCoordinate(const Vector3& position) const noexcept;
		float DotNormal(const Vector3& normal) const noexcept;

		// Static functions
		static void Transform(const Plane& plane, const Matrix& M, Plane& result) noexcept;
		static Plane Transform(const Plane& plane, const Matrix& M) noexcept;

		static void Transform(const Plane& plane, const Quaternion& rotation, Plane& result) noexcept;
		static Plane Transform(const Plane& plane, const Quaternion& rotation) noexcept;
		// Input quaternion must be the inverse transpose of the transformation
	};
}

