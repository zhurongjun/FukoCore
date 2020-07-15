#pragma once
#include "DirectXMath/DirectXMath.h"
using namespace DirectX;

// Color
namespace Fuko
{
	struct Vector2;
	struct Vector4;
	struct Matrix;
	struct Quaternion;
	struct Plane;

	struct Color : public XMFLOAT4
	{
		Color() noexcept : XMFLOAT4(0, 0, 0, 1.f) {}
		constexpr Color(float _r, float _g, float _b) noexcept : XMFLOAT4(_r, _g, _b, 1.f) {}
		constexpr Color(float _r, float _g, float _b, float _a) noexcept : XMFLOAT4(_r, _g, _b, _a) {}
		explicit Color(const Vector3& clr) noexcept : XMFLOAT4(clr.x, clr.y, clr.z, 1.f) {}
		explicit Color(const Vector4& clr) noexcept : XMFLOAT4(clr.x, clr.y, clr.z, clr.w) {}
		explicit Color(_In_reads_(4) const float *pArray) noexcept : XMFLOAT4(pArray) {}
		Color(FXMVECTOR V) noexcept { XMStoreFloat4(this, V); }
		Color(const XMFLOAT4& c) noexcept { this->x = c.x; this->y = c.y; this->z = c.z; this->w = c.w; }
		explicit Color(const XMVECTORF32& F) noexcept { this->x = F.f[0]; this->y = F.f[1]; this->z = F.f[2]; this->w = F.f[3]; }

		explicit Color(const DirectX::PackedVector::XMCOLOR& Packed) noexcept;
		// BGRA Direct3D 9 D3DCOLOR packed color

		explicit Color(const DirectX::PackedVector::XMUBYTEN4& Packed) noexcept;
		// RGBA XNA Game Studio packed color

		Color(const Color&) = default;
		Color& operator=(const Color&) = default;

		Color(Color&&) = default;
		Color& operator=(Color&&) = default;

		operator XMVECTOR() const noexcept { return XMLoadFloat4(this); }
		operator const float*() const noexcept { return reinterpret_cast<const float*>(this); }

		// Comparison operators
		bool operator == (const Color& c) const noexcept;
		bool operator != (const Color& c) const noexcept;

		// Assignment operators
		Color& operator= (const XMVECTORF32& F) noexcept { x = F.f[0]; y = F.f[1]; z = F.f[2]; w = F.f[3]; return *this; }
		Color& operator= (const DirectX::PackedVector::XMCOLOR& Packed) noexcept;
		Color& operator= (const DirectX::PackedVector::XMUBYTEN4& Packed) noexcept;
		Color& operator+= (const Color& c) noexcept;
		Color& operator-= (const Color& c) noexcept;
		Color& operator*= (const Color& c) noexcept;
		Color& operator*= (float S) noexcept;
		Color& operator/= (const Color& c) noexcept;

		// Unary operators
		Color operator+ () const noexcept { return *this; }
		Color operator- () const noexcept;

		// Properties
		float R() const noexcept { return x; }
		void R(float r) noexcept { x = r; }

		float G() const noexcept { return y; }
		void G(float g) noexcept { y = g; }

		float B() const noexcept { return z; }
		void B(float b) noexcept { z = b; }

		float A() const noexcept { return w; }
		void A(float a) noexcept { w = a; }

		// Color operations
		DirectX::PackedVector::XMCOLOR BGRA() const noexcept;
		DirectX::PackedVector::XMUBYTEN4 RGBA() const noexcept;

		Vector3 ToVector3() const noexcept;
		Vector4 ToVector4() const noexcept;

		void Negate() noexcept;
		void Negate(Color& result) const noexcept;

		void Saturate() noexcept;
		void Saturate(Color& result) const noexcept;

		void Premultiply() noexcept;
		void Premultiply(Color& result) const noexcept;

		void AdjustSaturation(float sat) noexcept;
		void AdjustSaturation(float sat, Color& result) const noexcept;

		void AdjustContrast(float contrast) noexcept;
		void AdjustContrast(float contrast, Color& result) const noexcept;

		// Static functions
		static void Modulate(const Color& c1, const Color& c2, Color& result) noexcept;
		static Color Modulate(const Color& c1, const Color& c2) noexcept;

		static void Lerp(const Color& c1, const Color& c2, float t, Color& result) noexcept;
		static Color Lerp(const Color& c1, const Color& c2, float t) noexcept;
	};

	// Binary operators
	Color operator+ (const Color& C1, const Color& C2) noexcept;
	Color operator- (const Color& C1, const Color& C2) noexcept;
	Color operator* (const Color& C1, const Color& C2) noexcept;
	Color operator* (const Color& C, float S) noexcept;
	Color operator/ (const Color& C1, const Color& C2) noexcept;
	Color operator* (float S, const Color& C) noexcept;
}

