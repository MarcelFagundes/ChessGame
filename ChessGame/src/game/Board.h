#pragma once
#include "Piece.h"
#include <array>

// row 0 = fileira 8 (pretas), row 7 = fileira 1 (brancas) — combina com a
// convencao de tela usada no BoardRenderer (linha 0 desenhada no topo).
struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
    bool isCapture = false;
};

class Board {
public:
    Board();

    void SetupInitialPosition();

    Piece& At(int row, int col) { return m_squares[row][col]; }
    const Piece& At(int row, int col) const { return m_squares[row][col]; }

    static bool IsInside(int row, int col) {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }

    // Aplica um movimento sem validar legalidade — a validacao acontece
    // antes, usando MoveGenerator::GenerateMoves.
    void ApplyMove(const Move& move);

private:
    std::array<std::array<Piece, 8>, 8> m_squares;
};