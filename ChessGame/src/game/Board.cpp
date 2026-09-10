#include "Board.h"

Board::Board() {
    SetupInitialPosition();
}

void Board::SetupInitialPosition() {
    for (auto& row : m_squares)
        for (auto& square : row)
            square = Piece{};

    const PieceType backRank[8] = {
        PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen,
        PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook
    };

    for (int col = 0; col < 8; ++col) {
        m_squares[0][col] = { backRank[col], PieceColor::Black };
        m_squares[1][col] = { PieceType::Pawn, PieceColor::Black };
        m_squares[6][col] = { PieceType::Pawn, PieceColor::White };
        m_squares[7][col] = { backRank[col], PieceColor::White };
    }
}

void Board::ApplyMove(const Move& move) {
    Piece& from = m_squares[move.fromRow][move.fromCol];
    Piece& to = m_squares[move.toRow][move.toCol];

    // Promocao simplificada: peao que alcanca a ultima fileira vira rainha
    // automaticamente. Escolher a peca de promocao fica como exercicio.
    if (from.type == PieceType::Pawn && (move.toRow == 0 || move.toRow == 7)) {
        to = { PieceType::Queen, from.color };
    } else {
        to = from;
    }
    from = Piece{};
}
