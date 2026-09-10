#include "MoveGenerator.h"
#include <utility>

namespace {

// Usado por Bispo, Torre e Rainha: anda em uma direcao ate encontrar
// borda do tabuleiro, peca propria (para) ou peca adversaria (captura e para).
void TryAddSlide(const Board& board, std::vector<Move>& moves, int row, int col,
                  int dRow, int dCol, PieceColor color) {
    int r = row + dRow;
    int c = col + dCol;
    while (Board::IsInside(r, c)) {
        const Piece& target = board.At(r, c);
        if (target.IsEmpty()) {
            moves.push_back({ row, col, r, c, false });
        } else {
            if (target.color != color) {
                moves.push_back({ row, col, r, c, true });
            }
            break;
        }
        r += dRow;
        c += dCol;
    }
}

// Usado por Cavalo e Rei: um unico passo fixo, sem deslizar.
void TryAddStep(const Board& board, std::vector<Move>& moves, int row, int col,
                 int toRow, int toCol, PieceColor color) {
    if (!Board::IsInside(toRow, toCol)) return;
    const Piece& target = board.At(toRow, toCol);
    if (target.IsEmpty() || target.color != color) {
        moves.push_back({ row, col, toRow, toCol, !target.IsEmpty() });
    }
}

void GeneratePawnMoves(const Board& board, std::vector<Move>& moves, int row, int col, PieceColor color) {
    int dir = (color == PieceColor::White) ? -1 : 1;   // brancas sobem (row diminui)
    int startRow = (color == PieceColor::White) ? 6 : 1;

    if (Board::IsInside(row + dir, col) && board.At(row + dir, col).IsEmpty()) {
        moves.push_back({ row, col, row + dir, col, false });

        if (row == startRow && board.At(row + 2 * dir, col).IsEmpty()) {
            moves.push_back({ row, col, row + 2 * dir, col, false });
        }
    }

    for (int dCol : { -1, 1 }) {
        int r = row + dir;
        int c = col + dCol;
        if (Board::IsInside(r, c)) {
            const Piece& target = board.At(r, c);
            if (!target.IsEmpty() && target.color != color) {
                moves.push_back({ row, col, r, c, true });
            }
        }
    }
    // En passant nao implementado nesta versao didatica.
}

} // namespace

std::vector<Move> MoveGenerator::GenerateMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    const Piece& piece = board.At(row, col);
    if (piece.IsEmpty()) return moves;

    switch (piece.type) {
        case PieceType::Pawn:
            GeneratePawnMoves(board, moves, row, col, piece.color);
            break;

        case PieceType::Knight: {
            const int offsets[8][2] = {
                {-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1}
            };
            for (auto& o : offsets)
                TryAddStep(board, moves, row, col, row + o[0], col + o[1], piece.color);
            break;
        }

        case PieceType::Bishop:
            for (auto [dr, dc] : { std::pair{-1,-1}, std::pair{-1,1}, std::pair{1,-1}, std::pair{1,1} })
                TryAddSlide(board, moves, row, col, dr, dc, piece.color);
            break;

        case PieceType::Rook:
            for (auto [dr, dc] : { std::pair{-1,0}, std::pair{1,0}, std::pair{0,-1}, std::pair{0,1} })
                TryAddSlide(board, moves, row, col, dr, dc, piece.color);
            break;

        case PieceType::Queen:
            for (auto [dr, dc] : { std::pair{-1,-1}, std::pair{-1,1}, std::pair{1,-1}, std::pair{1,1},
                                    std::pair{-1,0}, std::pair{1,0}, std::pair{0,-1}, std::pair{0,1} })
                TryAddSlide(board, moves, row, col, dr, dc, piece.color);
            break;

        case PieceType::King:
            for (int dr = -1; dr <= 1; ++dr)
                for (int dc = -1; dc <= 1; ++dc)
                    if (dr != 0 || dc != 0)
                        TryAddStep(board, moves, row, col, row + dr, col + dc, piece.color);
            // Roque nao implementado nesta versao didatica.
            break;

        default:
            break;
    }

    return moves;
}
