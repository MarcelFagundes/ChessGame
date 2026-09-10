#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <utility>

using Microsoft::WRL::ComPtr;

// Desenha os 64 quadrados do tabuleiro (geometria estatica) mais os
// destaques dinamicos: casa selecionada e casas de destino legais.
class BoardRenderer {
public:
    bool Initialize(ID3D11Device* device);
    void Draw(ID3D11DeviceContext* context, int selectedRow, int selectedCol,
              const std::vector<std::pair<int, int>>& legalTargets);

private:
    struct Vertex { float x, y; float r, g, b, a; };

    bool CreateShaders(ID3D11Device* device);
    bool CreateBoardGeometry(ID3D11Device* device);

    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11BlendState> m_blendState;

    ComPtr<ID3D11Buffer> m_boardVertexBuffer;
    ComPtr<ID3D11Buffer> m_boardIndexBuffer;
    UINT m_boardIndexCount = 0;

    ComPtr<ID3D11Buffer> m_overlayVertexBuffer;
    ComPtr<ID3D11Buffer> m_overlayIndexBuffer;
};
