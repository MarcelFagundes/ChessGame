#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "../engine/TextAtlas.h"
#include "../game/Board.h"

using Microsoft::WRL::ComPtr;

// Desenha as pecas presentes no tabuleiro como quads texturizados,
// amostrando o atlas de letras gerado por TextAtlas.
class PieceRenderer {
public:
    bool Initialize(ID3D11Device* device);
    void Draw(ID3D11DeviceContext* context, const Board& board);

private:
    struct Vertex { float x, y; float u, v; float r, g, b; };

    GlyphIndex GlyphForType(PieceType type) const;

    TextAtlas m_atlas;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11SamplerState> m_sampler;
    ComPtr<ID3D11BlendState> m_blendState;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;
};
