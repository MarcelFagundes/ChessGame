#include "PieceRenderer.h"
#include "../engine/ShaderUtils.h"
#include <vector>


GlyphIndex PieceRenderer::GlyphForType(PieceType type) const {
    switch (type) {
        case PieceType::Pawn:   return GlyphIndex::Pawn;
        case PieceType::Knight: return GlyphIndex::Knight;
        case PieceType::Bishop: return GlyphIndex::Bishop;
        case PieceType::Rook:   return GlyphIndex::Rook;
        case PieceType::Queen:  return GlyphIndex::Queen;
        case PieceType::King:   return GlyphIndex::King;
        default:                return GlyphIndex::Pawn;
    }
}

bool PieceRenderer::Initialize(ID3D11Device* device) {
    if (!m_atlas.Create(device)) return false;

    ComPtr<ID3DBlob> vsBlob, psBlob;
    if (!CompileShaderFromFile(L"Shaders/PieceVS.hlsl", "main", "vs_5_0", vsBlob)) return false;
    if (!CompileShaderFromFile(L"Shaders/PiecePS.hlsl", "main", "ps_5_0", psBlob)) return false;

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    device->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device->CreateSamplerState(&sampDesc, &m_sampler);

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendDesc, &m_blendState);

    // Buffers dinamicos: ate 32 pecas simultaneas no tabuleiro.
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DYNAMIC;
    vbDesc.ByteWidth = sizeof(Vertex) * 4 * 32;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&vbDesc, nullptr, &m_vertexBuffer);

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DYNAMIC;
    ibDesc.ByteWidth = sizeof(UINT) * 6 * 32;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&ibDesc, nullptr, &m_indexBuffer);

    return true;
}

void PieceRenderer::Draw(ID3D11DeviceContext* context, const Board& board) {
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;


    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Piece& piece = board.At(row, col);
            if (piece.IsEmpty()) continue;

            float u0, v0, u1, v1;
            m_atlas.GetUV(GlyphForType(piece.type), u0, v0, u1, v1);

            // Peca branca = quase branco; peca preta = quase preto.
            // (Nao 100% puro para nao se perder totalmente contra o fundo.)
            float shade = (piece.color == PieceColor::White) ? 0.97f : 0.06f;

            UINT base = static_cast<UINT>(vertices.size());
            vertices.push_back({ (float)col,        (float)row,        u0, v0, shade, shade, shade });
            vertices.push_back({ (float)col + 1.0f,  (float)row,        u1, v0, shade, shade, shade });
            vertices.push_back({ (float)col,        (float)row + 1.0f,  u0, v1, shade, shade, shade });
            vertices.push_back({ (float)col + 1.0f,  (float)row + 1.0f,  u1, v1, shade, shade, shade });

            indices.insert(indices.end(), { base, base + 1, base + 2, base + 1, base + 3, base + 2 });
        }
    }

    if (vertices.empty()) return;

    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, vertices.data(), vertices.size() * sizeof(Vertex));
    context->Unmap(m_vertexBuffer.Get(), 0);

    context->Map(m_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, indices.data(), indices.size() * sizeof(UINT));
    context->Unmap(m_indexBuffer.Get(), 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11ShaderResourceView* srv = m_atlas.GetSRV();

    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->PSSetShaderResources(0, 1, &srv);
    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
    context->OMSetBlendState(m_blendState.Get(), nullptr, 0xffffffff);

    context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}
