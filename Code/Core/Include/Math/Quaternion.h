#pragma once
#include "DirectXMath/DirectXMath.h"
using namespace DirectX;

// Quaternion
namespace Fuko
{
	struct Vector2;
	struct Vector4;
	struct Matrix;
	struct Quaternion;
	struct Plane;

	// Quaternion
	struct Quaternion : public XMFLOAT4
	{
		Quaternion() noexcept : XMFLOAT4(0, 0, 0, 1.f) {}
		constexpr Quaternion(float ix, float iy, float iz, float iw) noexcept : XMFLOAT4(ix, iy, iz, iw) {}
		Quaternion(const Vector3& v, float scalar) noexcept : XMFLOAT4(v.x, v.y, v.z, scalar) {}
		explicit Quaternion(const Vector4& v) noexcept : XMFLOAT4(v.x, v.y, v.z, v.w) {}
		explicit Quaternion(_In_reads_(4) const float *pArray) noexcept : XMFLOAT4(pArray) {}
		Quaternion(FXMVECTOR V) noexcept { XMStoreFloat4(this, V); }
		Quaternion(const XMFLOAT4& q) noexcept { this->x = q.x; this->y = q.y; this->z = q.z; this->w = q.w; }
		explicit Quaternion(const XMVECTORF32& F) noexcept { this->x = F.f[0]; this->y = F.f[1]; this->z = F.f[2]; this->w = F.f[3]; }

		Quaternion(const Quaternion&) = default;
		Quaternion& operator=(const Quaternion&) = default;

		Quaternion(Quaternion&&) = default;
		Quaternion& operator=(Quaternion&&) = default;

		operator XMVECTOR() const noexcept { return XMLoadFloat4(this); }

		// Comparison operators
		bool operator == (const Quaternion& q) const noexcept;
		bool operator != (const Quaternion& q) const noexcept;

		// Assignment operators
		Quaternion& operator= (const XMVECTORF32& F) noexcept { x = F.f[0]; y = F.f[1]; z = F.f[2]; w = F.f[3]; return *this; }
		Quaternion& operator+= (const Quaternion& q) noexcept;
		Quaternion& operator-= (const Quaternion& q) noexcept;
		Quaternion& operator*= (const Quaternion& q) noexcept;
		Quaternion& operator*= (float S) noexcept;
		Quaternion& operator/= (const Quaternion& q) noexcept;

		// Unary operators
		Quaternion operator+ () const  noexcept { return *this; }
		Quaternion operator- () const noexcept;

		// Quaternion operations
		float Length() const noexcept;
		float LengthSquared() const noexcept;

		void Normalize() noexcept;
		void Normalize(Quaternion& result) const noexcept;

		void Conjugate() noexcept;
		void Conjugate(Quaternion& result) const noexcept;

		void Inverse(Quaternion& result) const noexcept;

		float Dot(const Quaternion& Q) const noexcept;

		// Static functions
		static Quaternion CreateFromAxisAngle(const Vector3& axis, float angle) noexcept;
		static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept;
		static Quaternion CreateFromRotationMatrix(const Matrix& M) noexcept;

		static void Lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result) noexcept;
		static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept;

		static void Slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result) noexcept;
		static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept;

		static void Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result) noexcept;
		static Quaternion Concatenate(const Quaternion& q1, const Quaternion& q2) noexcept;

		// Constants
		static const Quaternion Identity;
	};

	// Binary operators
	Quaternion operator+ (const Quaternion& Q1, const Quaternion& Q2) noexcept;
	Quaternion operator- (const Quaternion& Q1, const Quaternion& Q2) noexcept;
	Quaternion operator* (const Quaternion& Q1, const Quaternion& Q2) noexcept;
	Quaternion operator* (const Quaternion& Q, float S) noexcept;
	Quaternion operator/ (const Quaternion& Q1, const Quaternion& Q2) noexcept;
	Quaternion operator* (float S, const Quaternion& Q) noexcept;
}


