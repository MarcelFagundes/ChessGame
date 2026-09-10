#include "engine/Window.h"
#include "engine/GraphicsDevice.h"
#include "engine/Timer.h"
#include "render/BoardRenderer.h"
#include "render/PieceRenderer.h"
#include "game/Board.h"
#include "game/MoveGenerator.h"

#include <utility>

namespace {

Board g_board;
PieceColor g_turn = PieceColor::White;
int g_selectedRow = -1;
int g_selectedCol = -1;
std::vector<Move> g_legalMoves;

void ClearSelection() {
    g_selectedRow = -1;
    g_selectedCol = -1;
    g_legalMoves.clear();
}

// Logica de clique: primeiro clique seleciona uma peca do jogador da vez
// e mostra os alvos legais; segundo clique, se for um alvo valido, move.
void OnBoardClick(int row, int col) {
    if (!Board::IsInside(row, col)) return;

    if (g_selectedRow >= 0) {
        for (const Move& move : g_legalMoves) {
            if (move.toRow == row && move.toCol == col) {
                g_board.ApplyMove(move);
                g_turn = (g_turn == PieceColor::White) ? PieceColor::Black : PieceColor::White;
                ClearSelection();
                return;
            }
        }
        // Clique fora dos alvos legais: cancela a selecao atual.
        ClearSelection();
    }

    const Piece& clicked = g_board.At(row, col);
    if (!clicked.IsEmpty() && clicked.color == g_turn) {
        g_selectedRow = row;
        g_selectedCol = col;
        g_legalMoves = MoveGenerator::GenerateMoves(g_board, row, col);
    }
}

} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {

    Window window;
    if (!window.Create(1024, 1024, L"Jogo de Xadrez Didatico - Game Chess Math")) {
        MessageBoxA(nullptr, "Falha 1: A janela (Window) nao pôde ser criada!", "Diagnostico de Inicializacao", MB_ICONERROR);
        return -1;
    }

    GraphicsDevice graphics;
    if (!graphics.Initialize(window.GetHandle(), window.GetWidth(), window.GetHeight())) {
        MessageBoxA(nullptr, "Falha 2: O DirectX 11 (GraphicsDevice) nao pôde ser inicializado!", "Diagnostico de Inicializacao", MB_ICONERROR);
        return -1;
    }

    BoardRenderer boardRenderer;
    if (!boardRenderer.Initialize(graphics.GetDevice())) return -1;

    PieceRenderer pieceRenderer;
    if (!pieceRenderer.Initialize(graphics.GetDevice())) return -1;

    window.OnResize = [&](int width, int height) {
        graphics.Resize(width, height);
    };

    window.OnMouseDown = [&](int x, int y) {
        int col = (x * 8) / (window.GetWidth() > 0 ? window.GetWidth() : 1);
        int row = (y * 8) / (window.GetHeight() > 0 ? window.GetHeight() : 1);
        OnBoardClick(row, col);
    };

    window.OnKeyDown = [&](int key) {
        if (key == VK_ESCAPE) PostQuitMessage(0);
        if (key == 'R') { g_board.SetupInitialPosition(); g_turn = PieceColor::White; ClearSelection(); }
    };

    Timer timer;
    bool running = true;
    while (running) {
        running = window.ProcessMessages();
        float deltaTime = timer.Tick();
        (void)deltaTime; // reservado para animacoes futuras (movimento suave de pecas)

        graphics.Clear(0.05f, 0.05f, 0.08f, 1.0f);

        std::vector<std::pair<int, int>> targets;
        targets.reserve(g_legalMoves.size());
        for (const Move& m : g_legalMoves) targets.push_back({ m.toRow, m.toCol });

        boardRenderer.Draw(graphics.GetContext(), g_selectedRow, g_selectedCol, targets);
        pieceRenderer.Draw(graphics.GetContext(), g_board);

        graphics.Present();
    }

    return 0;
}