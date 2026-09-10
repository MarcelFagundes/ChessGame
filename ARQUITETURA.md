# Arquitetura do Jogo de Xadrez Didático

Este documento apresenta o diagrama de classes UML do projeto, um diagrama
de sequência do fluxo de um lance completo, e a explicação de como as
peças se conectam em tempo de execução.

---

## 1. Diagrama de Classes

```mermaid
classDiagram
    class Main {
        <<wWinMain>>
        -Board g_board
        -PieceColor g_turn
        -int g_selectedRow
        -int g_selectedCol
        -vector~Move~ g_legalMoves
        +OnBoardClick(row, col) void
        +ClearSelection() void
    }

    class Window {
        -HWND m_hwnd
        -int m_width
        -int m_height
        +Create(width, height, title) bool
        +ProcessMessages() bool
        +GetHandle() HWND
        +OnResize : function~void(int,int)~
        +OnMouseDown : function~void(int,int)~
        +OnKeyDown : function~void(int)~
    }

    class GraphicsDevice {
        -ComPtr~ID3D11Device~ m_device
        -ComPtr~ID3D11DeviceContext~ m_context
        -ComPtr~IDXGISwapChain~ m_swapChain
        -ComPtr~ID3D11RenderTargetView~ m_renderTargetView
        +Initialize(hwnd, w, h) bool
        +Resize(w, h) void
        +Clear(r, g, b, a) void
        +Present() void
        +GetDevice() ID3D11Device*
        +GetContext() ID3D11DeviceContext*
    }

    class Timer {
        -LARGE_INTEGER m_frequency
        -LARGE_INTEGER m_lastTime
        +Tick() float
    }

    class TextAtlas {
        -ComPtr~ID3D11ShaderResourceView~ m_srv
        +Create(device) bool
        +GetUV(glyph, u0, v0, u1, v1) void
        +GetSRV() ID3D11ShaderResourceView*
    }

    class BoardRenderer {
        -ComPtr~ID3D11Buffer~ m_boardVertexBuffer
        -ComPtr~ID3D11Buffer~ m_boardIndexBuffer
        -ComPtr~ID3D11Buffer~ m_overlayVertexBuffer
        -ComPtr~ID3D11Buffer~ m_overlayIndexBuffer
        +Initialize(device) bool
        +Draw(context, selRow, selCol, legalTargets) void
    }

    class PieceRenderer {
        -TextAtlas m_atlas
        -ComPtr~ID3D11Buffer~ m_vertexBuffer
        -ComPtr~ID3D11Buffer~ m_indexBuffer
        +Initialize(device) bool
        +Draw(context, board) void
        -GlyphForType(type) GlyphIndex
    }

    class Piece {
        +PieceType type
        +PieceColor color
        +IsEmpty() bool
    }

    class Move {
        +int fromRow
        +int fromCol
        +int toRow
        +int toCol
        +bool isCapture
    }

    class Board {
        -array~array~Piece, 8~, 8~ m_squares
        +SetupInitialPosition() void
        +At(row, col) Piece&
        +ApplyMove(move) void
        +IsInside(row, col)$ bool
    }

    class MoveGenerator {
        +GenerateMoves(board, row, col)$ vector~Move~
    }

    class PieceType {
        <<enumeration>>
        None
        Pawn
        Knight
        Bishop
        Rook
        Queen
        King
    }

    class PieceColor {
        <<enumeration>>
        None
        White
        Black
    }

    %% ---- Orquestração (main.cpp) ----
    Main --> Window : cria + registra callbacks
    Main --> GraphicsDevice : cria + inicializa
    Main --> BoardRenderer : cria + chama Draw()
    Main --> PieceRenderer : cria + chama Draw()
    Main --> Timer : mede deltaTime
    Main --> Board : possui g_board
    Main --> MoveGenerator : chama GenerateMoves()
    Main --> Move : guarda g_legalMoves

    %% ---- Dependências de renderização ----
    PieceRenderer *-- TextAtlas : possui
    PieceRenderer ..> Board : lê para desenhar
    BoardRenderer ..> GraphicsDevice : usa ID3D11DeviceContext
    PieceRenderer ..> GraphicsDevice : usa ID3D11DeviceContext
    Window ..> GraphicsDevice : notifica Resize()

    %% ---- Regras do jogo ----
    Board "1" *-- "64" Piece : contém
    Piece --> PieceType
    Piece --> PieceColor
    MoveGenerator ..> Board : lê posição
    MoveGenerator ..> Move : gera lista
```

**Como ler as setas:** `-->` (seta cheia) indica que uma classe *possui ou
controla* a outra; `..>` (seta tracejada) indica uma *dependência de
leitura/uso*, sem posse; `*--` indica *composição* (o todo não existe sem
as partes — um `Board` sem `Piece` não faz sentido).

---

## 2. Diagrama de Sequência — um lance completo

```mermaid
sequenceDiagram
    participant OS as Windows (SO)
    participant Win as Window
    participant Main as main.cpp
    participant MG as MoveGenerator
    participant Board as Board
    participant BR as BoardRenderer
    participant PR as PieceRenderer
    participant GD as GraphicsDevice

    Note over OS,GD: Clique 1 — selecionar uma peça

    OS->>Win: WM_LBUTTONDOWN(x, y)
    Win->>Main: OnMouseDown(x, y)
    Main->>Main: converte (x,y) → (row, col)
    Main->>Main: OnBoardClick(row, col)
    Main->>Board: At(row, col)
    Board-->>Main: Piece (é sua e da vez)
    Main->>MG: GenerateMoves(board, row, col)
    MG-->>Main: vector<Move> (lances pseudo-legais)
    Main->>Main: g_selectedRow/Col + g_legalMoves

    Note over Main,GD: Frame seguinte — feedback visual

    Main->>Win: ProcessMessages()
    Main->>GD: Clear(cor de fundo)
    Main->>BR: Draw(context, selRow, selCol, targets)
    BR-->>GD: desenha 64 quadrados + destaques (amarelo/verde)
    Main->>PR: Draw(context, board)
    PR-->>GD: desenha as peças (quads texturizados)
    Main->>GD: Present()

    Note over OS,GD: Clique 2 — mover para um alvo legal

    OS->>Win: WM_LBUTTONDOWN(x2, y2)
    Win->>Main: OnMouseDown(x2, y2)
    Main->>Main: OnBoardClick(row2, col2)
    Main->>Main: row2,col2 está em g_legalMoves?
    Main->>Board: ApplyMove(move)
    Board-->>Board: move a peça, limpa origem,<br/>promove peão se aplicável
    Main->>Main: alterna g_turn (White ↔ Black)
    Main->>Main: ClearSelection()

    Note over Main,GD: Próximo frame redesenha tudo do zero,<br/>refletindo a nova posição
```

---

## 3. Explicação do fluxo, por etapa

### 3.1 Inicialização (uma única vez)

A ordem de criação em `wWinMain` é uma cadeia de dependências:

```
Window  →  GraphicsDevice  →  BoardRenderer / PieceRenderer
(precisa    (precisa do        (precisam do ID3D11Device
 de nada)    HWND da janela)    criado pelo GraphicsDevice)
```

Cada `Initialize()` só pode ser chamado depois que a etapa anterior
retornou `true`. Se qualquer uma falhar, o programa encerra com `-1` em
vez de continuar com ponteiros DirectX inválidos.

### 3.2 O game loop (repete a cada frame)

```
ProcessMessages()  →  Clear()  →  BoardRenderer::Draw()  →  PieceRenderer::Draw()  →  Present()
```

`ProcessMessages()` é o que faz a janela responder ao Windows — e é
*indiretamente*, através da fila de mensagens, que um clique do mouse
acaba chamando `OnBoardClick`. O `deltaTime` do `Timer` já é calculado
todo frame, mas ainda não é consumido por nada (fica reservado para
animações futuras de movimento).

### 3.3 A camada que nunca muda: `game/`

`Board`, `Piece`, `Move` e `MoveGenerator` não incluem nada de DirectX.
Eles representam o estado do xadrez como dados puros (uma matriz 8×8 de
`Piece`) e regras puras (funções que leem esse estado e devolvem listas
de `Move` válidos). Isso significa que, em teoria, você poderia escrever
testes automatizados para `MoveGenerator::GenerateMoves` sem nunca abrir
uma janela — a lógica do jogo é completamente independente de como ela é
desenhada.

### 3.4 A camada que traduz estado em pixels: `render/`

`BoardRenderer` e `PieceRenderer` nunca modificam `Board` — eles só o
leem (setas tracejadas `..>` no diagrama de classes). A cada frame, eles
reconstroem a geometria do zero a partir do estado atual:

- `BoardRenderer` sempre desenha os mesmos 64 quadrados (geometria
  estática, criada uma única vez), mas recalcula os destaques (seleção e
  alvos legais) a cada frame, porque esses mudam com a interação.
- `PieceRenderer` varre a matriz inteira do `Board` a cada frame e gera
  um quad texturizado para cada casa ocupada, usando as coordenadas UV
  que `TextAtlas` calcula para cada tipo de peça.

### 3.5 O ponto de encontro: `main.cpp`

`main.cpp` é a única parte do código que conhece **tanto** `game/`
quanto `render/` — é ele quem lê o clique, pergunta ao `MoveGenerator`
quais são os lances válidos, aplica o lance escolhido no `Board`, e then
passa esse mesmo `Board` para os renderizadores desenharem. Nem `game/`
nem `render/` se conhecem diretamente; `main.cpp` é a "cola" entre eles —
isso é o que o diagrama de classes expressa com `Main` sendo a única
classe com setas cheias (posse) apontando para praticamente tudo.

---

## 4. Onde ficam os "buracos" propositais

Como vimos em conversas anteriores, `MoveGenerator` gera apenas lances
**pseudo-legais**. No diagrama de sequência acima, isso corresponde ao
passo `MG-->>Main: vector<Move>` — nenhuma verificação de xeque acontece
ali. Implementar essa verificação adicionaria uma nova dependência no
diagrama: `MoveGenerator` (ou uma nova classe `LegalMoveFilter`) passaria
a depender de uma cópia temporária de `Board` para simular cada lance
antes de confirmá-lo como legal.
