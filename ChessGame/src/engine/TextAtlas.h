#pragma once
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

// Ordem das letras no atlas: P N B R Q K
enum class GlyphIndex { Pawn = 0, Knight, Bishop, Rook, Queen, King, Count };

// Gera, em tempo de execucao, uma textura contendo as letras das pecas
// (P, N, B, R, Q, K) desenhadas via GDI. E' uma alternativa didatica ao uso
// de sprites/imagens prontas: nao exige nenhum arquivo de asset externo.
// O canal alfa da textura guarda a "cobertura" do texto (0 = fundo,
// 255 = traco da letra), permitindo colorir a peca via vertex color no
// pixel shader (branco ou preto) sem precisar de dois atlases.
class TextAtlas {
public:
    bool Create(ID3D11Device* device);

    ID3D11ShaderResourceView* GetSRV() const { return m_srv.Get(); }
    void GetUV(GlyphIndex glyph, float& u0, float& v0, float& u1, float& v1) const;

private:
    ComPtr<ID3D11ShaderResourceView> m_srv;
    static constexpr int kCellSize = 128;
    static constexpr int kGlyphCount = 6;
};
