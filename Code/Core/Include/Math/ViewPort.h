#pragma once
#include "DirectXMath/DirectXMath.h"
using namespace DirectX;

// viewport
namespace Fuko
{
	struct Vector2;
	struct Vector4;
	struct Matrix;
	struct Quaternion;
	struct Plane;

	class Viewport
	{
	public:
		float x;
		float y;
		float width;
		float height;
		float minDepth;
		float maxDepth;

		Viewport() noexcept :
			x(0.f), y(0.f), width(0.f), height(0.f), minDepth(0.f), maxDepth(1.f) {}
		constexpr Viewport(float ix, float iy, float iw, float ih, float iminz = 0.f, float imaxz = 1.f) noexcept :
			x(ix), y(iy), width(iw), height(ih), minDepth(iminz), maxDepth(imaxz) {}
		explicit Viewport(const RECT& rct) noexcept :
			x(float(rct.left)), y(float(rct.top)),
			width(float(rct.right - rct.left)),
			height(float(rct.bottom - rct.top)),
			minDepth(0.f), maxDepth(1.f) {}

		Viewport(const Viewport&) = default;
		Viewport& operator=(const Viewport&) = default;

		Viewport(Viewport&&) = default;
		Viewport& operator=(Viewport&&) = default;

		// Comparison operators
		bool operator == (const Viewport& vp) const noexcept;
		bool operator != (const Viewport& vp) const noexcept;

		// Assignment operators
		Viewport& operator= (const RECT& rct) noexcept;

		// Viewport operations
		float AspectRatio() const noexcept;

		Vector3 Project(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const noexcept;
		void Project(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world, Vector3& result) const noexcept;

		Vector3 Unproject(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const noexcept;
		void Unproject(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world, Vector3& result) const noexcept;

		// Static methods
		static RECT __cdecl ComputeDisplayArea(DXGI_SCALING scaling, UINT backBufferWidth, UINT backBufferHeight, int outputWidth, int outputHeight) noexcept;
		static RECT __cdecl ComputeTitleSafeArea(UINT backBufferWidth, UINT backBufferHeight) noexcept;
	};
}
