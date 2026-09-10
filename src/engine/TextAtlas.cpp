#include "TextAtlas.h"
#include <windows.h>
#include <vector>

bool TextAtlas::Create(ID3D11Device* device) {
    const int cell = kCellSize;
    const int width = cell * kGlyphCount;
    const int height = cell;

    // 1. Cria uma DIB section: uma area de memoria que o GDI enxerga como
    //    um bitmap e que nos podemos ler diretamente como array de pixels.
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // negativo = top-down (linha 0 no topo)
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HDC screenDC = GetDC(nullptr);
    HDC memDC = CreateCompatibleDC(screenDC);
    HBITMAP bitmap = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ oldBitmap = SelectObject(memDC, bitmap);

    // 2. Desenha fundo preto e as letras em branco (a "luminancia" do
    //    branco vira o canal alfa depois).
    RECT full = { 0, 0, width, height };
    FillRect(memDC, &full, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

    HFONT font = CreateFontW(
        cell - 24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );
    HGDIOBJ oldFont = SelectObject(memDC, font);
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, RGB(255, 255, 255));

    const wchar_t letters[kGlyphCount] = { L'P', L'N', L'B', L'R', L'Q', L'K' };
    for (int i = 0; i < kGlyphCount; ++i) {
        RECT cellRect = { i * cell, 0, (i + 1) * cell, cell };
        DrawTextW(memDC, &letters[i], 1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    GdiFlush(); // garante que o GDI terminou de escrever em 'bits'

    // 3. Converte BGRA (formato do GDI) para RGBA com alfa = luminancia.
    //    Onde havia letra branca (255,255,255) -> alfa 255 (opaco).
    //    Onde havia fundo preto (0,0,0) -> alfa 0 (transparente).
    std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * 4);
    unsigned char* src = static_cast<unsigned char*>(bits);
    for (int p = 0; p < width * height; ++p) {
        unsigned char b = src[p * 4 + 0];
        unsigned char g = src[p * 4 + 1];
        unsigned char r = src[p * 4 + 2];
        unsigned char luminance = static_cast<unsigned char>((r + g + b) / 3);

        pixels[p * 4 + 0] = 255;
        pixels[p * 4 + 1] = 255;
        pixels[p * 4 + 2] = 255;
        pixels[p * 4 + 3] = luminance;
    }

    SelectObject(memDC, oldFont);
    SelectObject(memDC, oldBitmap);
    DeleteObject(font);
    DeleteObject(bitmap);
    DeleteDC(memDC);
    ReleaseDC(nullptr, screenDC);

    // 4. Sobe os pixels para uma textura D3D11 normal, com SRV para
    //    ser amostrada pelo pixel shader das pecas.
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = width * 4;

    ComPtr<ID3D11Texture2D> texture;
    if (FAILED(device->CreateTexture2D(&desc, &initData, &texture))) return false;
    if (FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, &m_srv))) return false;

    return true;
}

void TextAtlas::GetUV(GlyphIndex glyph, float& u0, float& v0, float& u1, float& v1) const {
    int index = static_cast<int>(glyph);
    u0 = static_cast<float>(index) / kGlyphCount;
    u1 = static_cast<float>(index + 1) / kGlyphCount;
    v0 = 0.0f;
    v1 = 1.0f;
}