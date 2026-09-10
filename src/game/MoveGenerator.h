#pragma once
#include "Board.h"
#include <vector>

class MoveGenerator {
public:
    // Gera os movimentos PSEUDO-legais de uma peca: respeita o padrao de
    // movimento de cada tipo e bloqueios por outras pecas, mas NAO verifica
    // se o proprio rei fica em xeque apos a jogada. Adicionar essa checagem
    // (simulando a jogada e verificando ataques ao rei) e' um otimo proximo
    // passo de aprendizado.
    static std::vector<Move> GenerateMoves(const Board& board, int row, int col);
};
