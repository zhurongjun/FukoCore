#pragma once
#include "DirectXMath/DirectXMath.h"
using namespace DirectX;

// 4D vector
namespace Fuko
{
	struct Vector2;
	struct Vector4;
	struct Matrix;
	struct Quaternion;
	struct Plane;

	struct Vector4 : public XMFLOAT4
	{
		Vector4() noexcept : XMFLOAT4(0.f, 0.f, 0.f, 0.f) {}
		constexpr explicit Vector4(float ix) noexcept : XMFLOAT4(ix, ix, ix, ix) {}
		constexpr Vector4(float ix, float iy, float iz, float iw) noexcept : XMFLOAT4(ix, iy, iz, iw) {}
		explicit Vector4(_In_reads_(4) const float *pArray) noexcept : XMFLOAT4(pArray) {}
		Vector4(FXMVECTOR V) noexcept { XMStoreFloat4(this, V); }
		Vector4(const XMFLOAT4& V) noexcept { this->x = V.x; this->y = V.y; this->z = V.z; this->w = V.w; }
		explicit Vector4(const XMVECTORF32& F) noexcept { this->x = F.f[0]; this->y = F.f[1]; this->z = F.f[2]; this->w = F.f[3]; }

		Vector4(const Vector4&) = default;
		Vector4& operator=(const Vector4&) = default;

		Vector4(Vector4&&) = default;
		Vector4& operator=(Vector4&&) = default;

		operator XMVECTOR() const  noexcept { return XMLoadFloat4(this); }

		// Comparison operators
		bool operator == (const Vector4& V) const noexcept;
		bool operator != (const Vector4& V) const noexcept;

		// Assignment operators
		Vector4& operator= (const XMVECTORF32& F) noexcept { x = F.f[0]; y = F.f[1]; z = F.f[2]; w = F.f[3]; return *this; }
		Vector4& operator+= (const Vector4& V) noexcept;
		Vector4& operator-= (const Vector4& V) noexcept;
		Vector4& operator*= (const Vector4& V) noexcept;
		Vector4& operator*= (float S) noexcept;
		Vector4& operator/= (float S) noexcept;

		// Unary operators
		Vector4 operator+ () const noexcept { return *this; }
		Vector4 operator- () const noexcept;

		// Vector operations
		bool InBounds(const Vector4& Bounds) const noexcept;

		float Length() const noexcept;
		float LengthSquared() const noexcept;

		float Dot(const Vector4& V) const noexcept;
		void Cross(const Vector4& v1, const Vector4& v2, Vector4& result) const noexcept;
		Vector4 Cross(const Vector4& v1, const Vector4& v2) const noexcept;

		void Normalize() noexcept;
		void Normalize(Vector4& result) const noexcept;

		void Clamp(const Vector4& vmin, const Vector4& vmax) noexcept;
		void Clamp(const Vector4& vmin, const Vector4& vmax, Vector4& result) const noexcept;

		// Static functions
		static float Distance(const Vector4& v1, const Vector4& v2) noexcept;
		static float DistanceSquared(const Vector4& v1, const Vector4& v2) noexcept;

		static void Min(const Vector4& v1, const Vector4& v2, Vector4& result) noexcept;
		static Vector4 Min(const Vector4& v1, const Vector4& v2) noexcept;

		static void Max(const Vector4& v1, const Vector4& v2, Vector4& result) noexcept;
		static Vector4 Max(const Vector4& v1, const Vector4& v2) noexcept;

		static void Lerp(const Vector4& v1, const Vector4& v2, float t, Vector4& result) noexcept;
		static Vector4 Lerp(const Vector4& v1, const Vector4& v2, float t) noexcept;

		static void SmoothStep(const Vector4& v1, const Vector4& v2, float t, Vector4& result) noexcept;
		static Vector4 SmoothStep(const Vector4& v1, const Vector4& v2, float t) noexcept;

		static void Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g, Vector4& result) noexcept;
		static Vector4 Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g) noexcept;

		static void CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t, Vector4& result) noexcept;
		static Vector4 CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t) noexcept;

		static void Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t, Vector4& result) noexcept;
		static Vector4 Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t) noexcept;

		static void Reflect(const Vector4& ivec, const Vector4& nvec, Vector4& result) noexcept;
		static Vector4 Reflect(const Vector4& ivec, const Vector4& nvec) noexcept;

		static void Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex, Vector4& result) noexcept;
		static Vector4 Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex) noexcept;

		static void Transform(const Vector2& v, const Quaternion& quat, Vector4& result) noexcept;
		static Vector4 Transform(const Vector2& v, const Quaternion& quat) noexcept;

		static void Transform(const Vector3& v, const Quaternion& quat, Vector4& result) noexcept;
		static Vector4 Transform(const Vector3& v, const Quaternion& quat) noexcept;

		static void Transform(const Vector4& v, const Quaternion& quat, Vector4& result) noexcept;
		static Vector4 Transform(const Vector4& v, const Quaternion& quat) noexcept;

		static void Transform(const Vector4& v, const Matrix& m, Vector4& result) noexcept;
		static Vector4 Transform(const Vector4& v, const Matrix& m) noexcept;
		static void Transform(_In_reads_(count) const Vector4* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector4* resultArray) noexcept;

		// Constants
		static const Vector4 Zero;
		static const Vector4 One;
		static const Vector4 UnitX;
		static const Vector4 UnitY;
		static const Vector4 UnitZ;
		static const Vector4 UnitW;
	};

	// Binary operators
	Vector4 operator+ (const Vector4& V1, const Vector4& V2) noexcept;
	Vector4 operator- (const Vector4& V1, const Vector4& V2) noexcept;
	Vector4 operator* (const Vector4& V1, const Vector4& V2) noexcept;
	Vector4 operator* (const Vector4& V, float S) noexcept;
	Vector4 operator/ (const Vector4& V1, const Vector4& V2) noexcept;
	Vector4 operator/ (const Vector4& V, float S) noexcept;
	Vector4 operator* (float S, const Vector4& V) noexcept;
}