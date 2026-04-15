/***************************************************************************
* SRenderSystemTypesDX11.cpp
*/

#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"
#include "Core/SException.h"


void SRenderTarget::Init(ID3D11Device* inDevice, DXGI_FORMAT inFormat)
{
    if (device == inDevice) return;

    device = inDevice;
    format = inFormat;

    S_TRY

    UINT formatSupport = 0;
    if (FAILED(device->CheckFormatSupport(format, &formatSupport)))
    {
        throw std::exception("Cannot check format support");
    }

    constexpr UINT32 required = D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_RENDER_TARGET;
    if ((formatSupport & required) != required)
    {
        throw std::exception("Wrong texture format");
    }

    S_CATCH{ S_THROW("SRenderTarget::Init()") }
}

void SRenderTarget::Shutdown()
{
    renderTargetView.Reset();
    shaderResourceView.Reset();
    renderTarget.Reset();
    size = SConst::ZeroSSize2;
}

void SRenderTarget::Create(const SSize2& inSize)
{
    if (size == inSize || !device) return;

    S_TRY

    CD3D11_TEXTURE2D_DESC renderTargetDesc(format,
        static_cast<UINT>(inSize.width), static_cast<UINT>(inSize.height),
        1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
        D3D11_USAGE_DEFAULT, 0, 1
    );

    if (FAILED(device->CreateTexture2D(&renderTargetDesc,
        nullptr, renderTarget.ReleaseAndGetAddressOf())))
    {
        throw std::exception("Cannot create render target");
    }

    CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D, format);

    if (FAILED(device->CreateRenderTargetView(
        renderTarget.Get(), &renderTargetViewDesc,
        renderTargetView.ReleaseAndGetAddressOf())))
    {
        throw std::exception("Cannot create render target RTV");
    }

    CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(D3D11_SRV_DIMENSION_TEXTURE2D, format);

    if (FAILED(device->CreateShaderResourceView(
        renderTarget.Get(), &shaderResourceViewDesc,
        shaderResourceView.ReleaseAndGetAddressOf())))
    {
        throw std::exception("Cannot create render target SRV");
    }

    size = inSize;

    S_CATCH{ S_THROW("SRenderTarget::Create()") }
}
