#pragma once
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

// Encapsula device, contexto, swap chain e render target do DirectX 11.
// Nenhuma outra classe do jogo deve criar esses objetos diretamente;
// todas recebem ID3D11Device*/ID3D11DeviceContext* a partir daqui.
class GraphicsDevice {
public:
    bool Initialize(HWND hwnd, int width, int height);
    void Resize(int width, int height);
    void Clear(float r, float g, float b, float a);
    void Present();

    ID3D11Device* GetDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_context.Get(); }

private:
    bool CreateRenderTarget();

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    HWND m_hwnd = nullptr;
};
