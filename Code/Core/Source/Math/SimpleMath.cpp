//-------------------------------------------------------------------------------------
// SimpleMath.cpp -- Simplified C++ Math wrapper for DirectXMath
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//-------------------------------------------------------------------------------------

#include <Math/SimpleMath.h>
#include <d3d12.h>
#include <algorithm>

/****************************************************************************
 *
 * Constants
 *
 ****************************************************************************/

namespace Fuko
{
	const Vector2 Vector2::Zero = { 0.f, 0.f };
	const Vector2 Vector2::One = { 1.f, 1.f };
	const Vector2 Vector2::UnitX = { 1.f, 0.f };
	const Vector2 Vector2::UnitY = { 0.f, 1.f };

	const Vector3 Vector3::Zero = { 0.f, 0.f, 0.f };
	const Vector3 Vector3::One = { 1.f, 1.f, 1.f };
	const Vector3 Vector3::UnitX = { 1.f, 0.f, 0.f };
	const Vector3 Vector3::UnitY = { 0.f, 1.f, 0.f };
	const Vector3 Vector3::UnitZ = { 0.f, 0.f, 1.f };
	const Vector3 Vector3::Up = { 0.f, 1.f, 0.f };
	const Vector3 Vector3::Down = { 0.f, -1.f, 0.f };
	const Vector3 Vector3::Right = { 1.f, 0.f, 0.f };
	const Vector3 Vector3::Left = { -1.f, 0.f, 0.f };
	const Vector3 Vector3::Forward = { 0.f, 0.f, -1.f };
	const Vector3 Vector3::Backward = { 0.f, 0.f, 1.f };

	const Vector4 Vector4::Zero = { 0.f, 0.f, 0.f, 0.f };
	const Vector4 Vector4::One = { 1.f, 1.f, 1.f, 1.f };
	const Vector4 Vector4::UnitX = { 1.f, 0.f, 0.f, 0.f };
	const Vector4 Vector4::UnitY = { 0.f, 1.f, 0.f, 0.f };
	const Vector4 Vector4::UnitZ = { 0.f, 0.f, 1.f, 0.f };
	const Vector4 Vector4::UnitW = { 0.f, 0.f, 0.f, 1.f };

	const Matrix Matrix::Identity = { 1.f, 0.f, 0.f, 0.f,
										0.f, 1.f, 0.f, 0.f,
										0.f, 0.f, 1.f, 0.f,
										0.f, 0.f, 0.f, 1.f };

	const Quaternion Quaternion::Identity = { 0.f, 0.f, 0.f, 1.f };
}


/****************************************************************************
 *
 * Viewport
 *
 ****************************************************************************/

RECT Fuko::Viewport::ComputeDisplayArea(DXGI_SCALING scaling, UINT backBufferWidth, UINT backBufferHeight, int outputWidth, int outputHeight) noexcept
{
    RECT rct = {};

    switch (int(scaling))
    {
        case DXGI_SCALING_STRETCH:
            // Output fills the entire window area
            rct.top = 0;
            rct.left = 0;
            rct.right = outputWidth;
            rct.bottom = outputHeight;
            break;

        case 2 /*DXGI_SCALING_ASPECT_RATIO_STRETCH*/:
            // Output fills the window area but respects the original aspect ratio, using pillar boxing or letter boxing as required
            // Note: This scaling option is not supported for legacy Win32 windows swap chains
        {
            assert(backBufferHeight > 0);
            float aspectRatio = float(backBufferWidth) / float(backBufferHeight);

            // Horizontal fill
            float scaledWidth = float(outputWidth);
            float scaledHeight = float(outputWidth) / aspectRatio;
            if (scaledHeight >= float(outputHeight))
            {
                // Do vertical fill
                scaledWidth = float(outputHeight) * aspectRatio;
                scaledHeight = float(outputHeight);
            }

            float offsetX = (float(outputWidth) - scaledWidth) * 0.5f;
            float offsetY = (float(outputHeight) - scaledHeight) * 0.5f;

            rct.left = static_cast<LONG>(offsetX);
            rct.top = static_cast<LONG>(offsetY);
            rct.right = static_cast<LONG>(offsetX + scaledWidth);
            rct.bottom = static_cast<LONG>(offsetY + scaledHeight);

            // Clip to display window
            rct.left = std::max<LONG>(0, rct.left);
            rct.top = std::max<LONG>(0, rct.top);
            rct.right = std::min<LONG>(outputWidth, rct.right);
            rct.bottom = std::min<LONG>(outputHeight, rct.bottom);
        }
        break;

        case DXGI_SCALING_NONE:
        default:
            // Output is displayed in the upper left corner of the window area
            rct.top = 0;
            rct.left = 0;
            rct.right = std::min<LONG>(static_cast<LONG>(backBufferWidth), outputWidth);
            rct.bottom = std::min<LONG>(static_cast<LONG>(backBufferHeight), outputHeight);
            break;
    }

    return rct;
}

RECT Fuko::Viewport::ComputeTitleSafeArea(UINT backBufferWidth, UINT backBufferHeight) noexcept
{
    float safew = (float(backBufferWidth) + 19.f) / 20.f;
    float safeh = (float(backBufferHeight) + 19.f) / 20.f;

    RECT rct;
    rct.left = static_cast<LONG>(safew);
    rct.top = static_cast<LONG>(safeh);
    rct.right = static_cast<LONG>(float(backBufferWidth) - safew + 0.5f);
    rct.bottom = static_cast<LONG>(float(backBufferHeight) - safeh + 0.5f);

    return rct;
}
