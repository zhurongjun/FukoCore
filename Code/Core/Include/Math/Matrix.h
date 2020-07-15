#pragma once
#include "DirectXMath/DirectXMath.h"
using namespace DirectX;

// 4x4 matrix 
namespace Fuko
{
	struct Vector2;
	struct Vector4;
	struct Matrix;
	struct Quaternion;
	struct Plane;

	struct Matrix : public XMFLOAT4X4
	{
		Matrix() noexcept
			: XMFLOAT4X4(1.f, 0, 0, 0,
						 0, 1.f, 0, 0,
						 0, 0, 1.f, 0,
						 0, 0, 0, 1.f) {}
		constexpr Matrix(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33) noexcept
			: XMFLOAT4X4(m00, m01, m02, m03,
						 m10, m11, m12, m13,
						 m20, m21, m22, m23,
						 m30, m31, m32, m33) {}
		explicit Matrix(const Vector3& r0, const Vector3& r1, const Vector3& r2) noexcept
			: XMFLOAT4X4(r0.x, r0.y, r0.z, 0,
						 r1.x, r1.y, r1.z, 0,
						 r2.x, r2.y, r2.z, 0,
						 0, 0, 0, 1.f) {}
		explicit Matrix(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3) noexcept
			: XMFLOAT4X4(r0.x, r0.y, r0.z, r0.w,
						 r1.x, r1.y, r1.z, r1.w,
						 r2.x, r2.y, r2.z, r2.w,
						 r3.x, r3.y, r3.z, r3.w) {}
		Matrix(const XMFLOAT4X4& M) noexcept { memcpy_s(this, sizeof(float) * 16, &M, sizeof(XMFLOAT4X4)); }
		Matrix(const XMFLOAT3X3& M) noexcept;
		Matrix(const XMFLOAT4X3& M) noexcept;

		explicit Matrix(_In_reads_(16) const float *pArray) noexcept : XMFLOAT4X4(pArray) {}
		Matrix(CXMMATRIX M) noexcept { XMStoreFloat4x4(this, M); }

		Matrix(const Matrix&) = default;
		Matrix& operator=(const Matrix&) = default;

		Matrix(Matrix&&) = default;
		Matrix& operator=(Matrix&&) = default;

		operator XMMATRIX() const noexcept { return XMLoadFloat4x4(this); }

		// Comparison operators
		bool operator == (const Matrix& M) const noexcept;
		bool operator != (const Matrix& M) const noexcept;

		// Assignment operators
		Matrix& operator= (const XMFLOAT3X3& M) noexcept;
		Matrix& operator= (const XMFLOAT4X3& M) noexcept;
		Matrix& operator+= (const Matrix& M) noexcept;
		Matrix& operator-= (const Matrix& M) noexcept;
		Matrix& operator*= (const Matrix& M) noexcept;
		Matrix& operator*= (float S) noexcept;
		Matrix& operator/= (float S) noexcept;

		Matrix& operator/= (const Matrix& M) noexcept;
		// Element-wise divide

		// Unary operators
		Matrix operator+ () const noexcept { return *this; }
		Matrix operator- () const noexcept;

		// Properties
		Vector3 Up() const noexcept { return Vector3(_21, _22, _23); }
		void Up(const Vector3& v) noexcept { _21 = v.x; _22 = v.y; _23 = v.z; }

		Vector3 Down() const  noexcept { return Vector3(-_21, -_22, -_23); }
		void Down(const Vector3& v) noexcept { _21 = -v.x; _22 = -v.y; _23 = -v.z; }

		Vector3 Right() const noexcept { return Vector3(_11, _12, _13); }
		void Right(const Vector3& v) noexcept { _11 = v.x; _12 = v.y; _13 = v.z; }

		Vector3 Left() const noexcept { return Vector3(-_11, -_12, -_13); }
		void Left(const Vector3& v) noexcept { _11 = -v.x; _12 = -v.y; _13 = -v.z; }

		Vector3 Forward() const noexcept { return Vector3(-_31, -_32, -_33); }
		void Forward(const Vector3& v) noexcept { _31 = -v.x; _32 = -v.y; _33 = -v.z; }

		Vector3 Backward() const noexcept { return Vector3(_31, _32, _33); }
		void Backward(const Vector3& v) noexcept { _31 = v.x; _32 = v.y; _33 = v.z; }

		Vector3 Translation() const  noexcept { return Vector3(_41, _42, _43); }
		void Translation(const Vector3& v) noexcept { _41 = v.x; _42 = v.y; _43 = v.z; }

		// Matrix operations
		bool Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation) noexcept;

		Matrix Transpose() const noexcept;
		void Transpose(Matrix& result) const noexcept;

		Matrix Invert() const noexcept;
		void Invert(Matrix& result) const noexcept;

		float Determinant() const noexcept;

		// Static functions
		static Matrix CreateBillboard(
			const Vector3& object, const Vector3& cameraPosition, const Vector3& cameraUp, _In_opt_ const Vector3* cameraForward = nullptr) noexcept;

		static Matrix CreateConstrainedBillboard(
			const Vector3& object, const Vector3& cameraPosition, const Vector3& rotateAxis,
			_In_opt_ const Vector3* cameraForward = nullptr, _In_opt_ const Vector3* objectForward = nullptr) noexcept;

		static Matrix CreateTranslation(const Vector3& position) noexcept;
		static Matrix CreateTranslation(float x, float y, float z) noexcept;

		static Matrix CreateScale(const Vector3& scales) noexcept;
		static Matrix CreateScale(float xs, float ys, float zs) noexcept;
		static Matrix CreateScale(float scale) noexcept;

		static Matrix CreateRotationX(float radians) noexcept;
		static Matrix CreateRotationY(float radians) noexcept;
		static Matrix CreateRotationZ(float radians) noexcept;

		static Matrix CreateFromAxisAngle(const Vector3& axis, float angle) noexcept;

		static Matrix CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane) noexcept;
		static Matrix CreatePerspective(float width, float height, float nearPlane, float farPlane) noexcept;
		static Matrix CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane) noexcept;
		static Matrix CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane) noexcept;
		static Matrix CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept;

		static Matrix CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up) noexcept;
		static Matrix CreateWorld(const Vector3& position, const Vector3& forward, const Vector3& up) noexcept;

		static Matrix CreateFromQuaternion(const Quaternion& quat) noexcept;

		static Matrix CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept;

		static Matrix CreateShadow(const Vector3& lightDir, const Plane& plane) noexcept;

		static Matrix CreateReflection(const Plane& plane) noexcept;

		static void Lerp(const Matrix& M1, const Matrix& M2, float t, Matrix& result) noexcept;
		static Matrix Lerp(const Matrix& M1, const Matrix& M2, float t) noexcept;

		static void Transform(const Matrix& M, const Quaternion& rotation, Matrix& result) noexcept;
		static Matrix Transform(const Matrix& M, const Quaternion& rotation) noexcept;

		// Constants
		static const Matrix Identity;
	};

	// Binary operators
	Matrix operator+ (const Matrix& M1, const Matrix& M2) noexcept;
	Matrix operator- (const Matrix& M1, const Matrix& M2) noexcept;
	Matrix operator* (const Matrix& M1, const Matrix& M2) noexcept;
	Matrix operator* (const Matrix& M, float S) noexcept;
	Matrix operator/ (const Matrix& M, float S) noexcept;
	Matrix operator/ (const Matrix& M1, const Matrix& M2) noexcept;
	// Element-wise divide
	Matrix operator* (float S, const Matrix& M) noexcept;
}
