#include "BoardRenderer.h"
#include "../engine/ShaderUtils.h"
#include <cstring>

bool BoardRenderer::Initialize(ID3D11Device* device) {
    return CreateShaders(device) && CreateBoardGeometry(device);
}

bool BoardRenderer::CreateShaders(ID3D11Device* device) {
    ComPtr<ID3DBlob> vsBlob, psBlob;
    if (!CompileShaderFromFile(L"Shaders/BoardVS.hlsl", "main", "vs_5_0", vsBlob)) return false;
    if (!CompileShaderFromFile(L"Shaders/BoardPS.hlsl", "main", "ps_5_0", psBlob)) return false;

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

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

    return true;
}

bool BoardRenderer::CreateBoardGeometry(ID3D11Device* device) {
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    const float light[4] = { 0.93f, 0.86f, 0.73f, 1.0f };
    const float dark[4]  = { 0.35f, 0.25f, 0.18f, 1.0f };

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const float* c = ((row + col) % 2 == 0) ? light : dark;
            UINT base = static_cast<UINT>(vertices.size());

            vertices.push_back({ (float)col,     (float)row,     c[0], c[1], c[2], c[3] });
            vertices.push_back({ (float)col + 1, (float)row,     c[0], c[1], c[2], c[3] });
            vertices.push_back({ (float)col,     (float)row + 1, c[0], c[1], c[2], c[3] });
            vertices.push_back({ (float)col + 1, (float)row + 1, c[0], c[1], c[2], c[3] });

            indices.insert(indices.end(), {
                base, base + 1, base + 2,
                base + 1, base + 3, base + 2
            });
        }
    }
    m_boardIndexCount = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex));
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = { vertices.data() };
    device->CreateBuffer(&vbDesc, &vbData, &m_boardVertexBuffer);

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
    ibDesc.ByteWidth = static_cast<UINT>(indices.size() * sizeof(UINT));
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA ibData = { indices.data() };
    device->CreateBuffer(&ibDesc, &ibData, &m_boardIndexBuffer);

    // Buffers DINAMICOS para os destaques: sao reescritos a cada frame
    // porque a selecao e as jogadas legais mudam com a interacao do jogador.
    // Tamanho maximo: rei tem no maximo 8 alvos, mas deixamos folga (32 quads).
    D3D11_BUFFER_DESC dynVbDesc = {};
    dynVbDesc.Usage = D3D11_USAGE_DYNAMIC;
    dynVbDesc.ByteWidth = sizeof(Vertex) * 4 * 32;
    dynVbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    dynVbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&dynVbDesc, nullptr, &m_overlayVertexBuffer);

    D3D11_BUFFER_DESC dynIbDesc = {};
    dynIbDesc.Usage = D3D11_USAGE_DYNAMIC;
    dynIbDesc.ByteWidth = sizeof(UINT) * 6 * 32;
    dynIbDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    dynIbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&dynIbDesc, nullptr, &m_overlayIndexBuffer);

    return true;
}

void BoardRenderer::Draw(ID3D11DeviceContext* context, int selectedRow, int selectedCol,
                          const std::vector<std::pair<int, int>>& legalTargets) {
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->OMSetBlendState(m_blendState.Get(), nullptr, 0xffffffff);

    // 1. Os 64 quadrados do tabuleiro (geometria estatica).
    context->IASetVertexBuffers(0, 1, m_boardVertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_boardIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(m_boardIndexCount, 0, 0);

    // 2. Destaques: casa selecionada (amarelo) + jogadas legais (verde),
    //    como quads translucidos sobrepostos ao tabuleiro.
    std::vector<Vertex> overlayVerts;
    std::vector<UINT> overlayIdx;

    auto addHighlight = [&](int row, int col, float r, float g, float b, float a) {
        UINT base = static_cast<UINT>(overlayVerts.size());
        overlayVerts.push_back({ (float)col,     (float)row,     r, g, b, a });
        overlayVerts.push_back({ (float)col + 1, (float)row,     r, g, b, a });
        overlayVerts.push_back({ (float)col,     (float)row + 1, r, g, b, a });
        overlayVerts.push_back({ (float)col + 1, (float)row + 1, r, g, b, a });
        overlayIdx.insert(overlayIdx.end(), { base, base + 1, base + 2, base + 1, base + 3, base + 2 });
    };

    if (selectedRow >= 0) addHighlight(selectedRow, selectedCol, 0.95f, 0.85f, 0.2f, 0.5f);
    for (auto& target : legalTargets) addHighlight(target.first, target.second, 0.2f, 0.8f, 0.3f, 0.45f);

    if (!overlayVerts.empty()) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        context->Map(m_overlayVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, overlayVerts.data(), overlayVerts.size() * sizeof(Vertex));
        context->Unmap(m_overlayVertexBuffer.Get(), 0);

        context->Map(m_overlayIndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, overlayIdx.data(), overlayIdx.size() * sizeof(UINT));
        context->Unmap(m_overlayIndexBuffer.Get(), 0);

        context->IASetVertexBuffers(0, 1, m_overlayVertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(m_overlayIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(static_cast<UINT>(overlayIdx.size()), 0, 0);
    }
}
