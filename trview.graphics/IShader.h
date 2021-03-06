#pragma once

#include <wrl/client.h>
#include <d3d11.h>

namespace trview
{
    namespace graphics
    {
        struct IShader
        {
            virtual ~IShader() = 0;

            virtual void apply(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context) = 0;
        };
    }
}

