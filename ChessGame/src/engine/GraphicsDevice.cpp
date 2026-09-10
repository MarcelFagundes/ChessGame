#include "GraphicsDevice.h"

bool GraphicsDevice::Initialize(HWND hwnd, int width, int height) {
    m_hwnd = hwnd;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT flags = 0;
#ifdef _DEBUG
    // Ativa a debug layer: erros de API aparecem detalhados na janela Output
    // do Visual Studio, em vez de a tela simplesmente nao renderizar nada.
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        nullptr, 0, D3D11_SDK_VERSION,
        &scd, &m_swapChain, &m_device, &featureLevel, &m_context
    );

    if (FAILED(hr)) return false;

    return CreateRenderTarget();
}

bool GraphicsDevice::CreateRenderTarget() {
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) return false;

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) return false;

    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(desc.Width);
    viewport.Height = static_cast<float>(desc.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &viewport);

    return true;
}

void GraphicsDevice::Resize(int width, int height) {
    if (!m_swapChain || width == 0 || height == 0) return;

    // O render target antigo precisa ser liberado ANTES do ResizeBuffers,
    // senao o DirectX recusa o resize com erro de "recurso ainda em uso".
    m_context->OMSetRenderTargets(0, nullptr, nullptr);
    m_renderTargetView.Reset();

    m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();
}

void GraphicsDevice::Clear(float r, float g, float b, float a) {
    const float color[4] = { r, g, b, a };
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), color);
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
}

void GraphicsDevice::Present() {
    m_swapChain->Present(1, 0); // 1 = espera o vsync, evita tearing
}
