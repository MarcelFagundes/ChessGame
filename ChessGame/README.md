# Jogo de Xadrez Didático — C++20 / Win32 / DirectX 11

Projeto de aprendizado de programação de jogos: um motor mínimo próprio
(janela, dispositivo gráfico, game loop) e um jogo de xadrez jogável
construído em cima dele, seguindo a separação clássica de camadas
`engine` / `game` / `render`.

## Como compilar (Visual Studio 2022)

1. Abra o **Visual Studio 2022** → "Abrir uma pasta local" → selecione a
   pasta `ChessGame` (a que contém o `CMakeLists.txt`). O Visual Studio
   detecta o CMake automaticamente e configura o projeto.
2. Espere a barra de status terminar de gerar o cache do CMake.
3. No seletor de configuração (topo), escolha `ChessGame.exe` como item de
   inicialização, se não estiver selecionado.
4. Pressione **F5**. Uma janela 1024×1024 deve abrir com o tabuleiro e as
   peças posicionadas para o início de uma partida.

Se preferir compilar via linha de comando (Developer PowerShell):

```powershell
cmake -B build -S .
cmake --build build --config Debug
```

O executável e a pasta `Shaders` (copiada automaticamente pelo CMake)
ficam em `build/Debug/`.

## Como jogar

- Clique em uma peça sua para selecioná-la — as casas de destino legais
  aparecem destacadas em verde.
- Clique em uma casa verde para mover, ou em qualquer outro lugar para
  cancelar a seleção.
- As brancas jogam primeiro; os turnos alternam automaticamente.
- Tecla **R** reinicia a partida. **Esc** fecha o jogo.

## Arquitetura

```
src/
├── main.cpp              # ponto de entrada: game loop, input, turnos
├── engine/                # nada aqui sabe o que é "xadrez"
│   ├── Window             # janela Win32, despacha eventos via std::function
│   ├── GraphicsDevice      # device/contexto/swapchain do D3D11
│   ├── Timer               # delta time (QueryPerformanceCounter)
│   ├── ShaderUtils         # compilação de .hlsl em tempo de execução
│   └── TextAtlas           # gera, via GDI, uma textura com as letras
│                            # P N B R Q K, usada para desenhar as peças
├── game/                   # nada aqui sabe o que é DirectX
│   ├── Piece                # enums PieceType / PieceColor
│   ├── Board                 # matriz 8x8, posição inicial, aplica jogadas
│   └── MoveGenerator          # gera movimentos por tipo de peça
└── render/                  # conecta game + engine
    ├── BoardRenderer          # desenha os 64 quadrados + destaques
    ├── PieceRenderer           # desenha as peças usando o TextAtlas
    └── Shaders/                 # BoardVS/PS.hlsl, PieceVS/PS.hlsl
```

A regra que guiou o design: **`game/` não inclui `<d3d11.h>` em lugar
nenhum**. Toda a lógica de xadrez pode, em princípio, ser testada com
testes unitários simples, sem abrir nenhuma janela — só depois ela é
"traduzida" visualmente pela camada `render/`.

## Limitações conhecidas (propositais — são os próximos passos de aprendizado)

Esta é uma versão **didática**, focada em you conseguir enxergar o motor
inteiro de ponta a ponta. Ficaram de fora, de propósito, para você
implementar como exercício:

- **Detecção de xeque e xeque-mate** — hoje `MoveGenerator` gera jogadas
  *pseudo-legais*: respeita o padrão de movimento de cada peça, mas não
  impede que você deixe o próprio rei em xeque. Para corrigir: simule cada
  jogada candidata em uma cópia do tabuleiro e descarte as que deixam seu
  rei atacado.
- **Roque** (castling).
- **En passant**.
- **Escolha da peça de promoção** — hoje todo peão que chega na última
  fileira vira rainha automaticamente.
- **Empate por afogamento, repetição ou regra dos 50 lances.**
- **Desfazer jogada (undo).**
- **Efeitos sonoros e animação suave de movimento** — o `Timer` já está
  no lugar para isso, mas não é usado ainda (`deltaTime` é descartado no
  `main.cpp`).
- **Suporte a texturas de imagem reais** para as peças, em vez do atlas de
  letras gerado por GDI (basta trocar `TextAtlas` por um carregador de PNG
  e ajustar `PieceRenderer` para usar suas UVs).

## Créditos e originalidade

Este código foi escrito do zero para fins de estudo, inspirado pela
**arquitetura geral** de motor 2D ensinada no curso *Programação de
Jogos* do professor Judson Santiago (UFERSA) — janela Win32 + DirectX 11,
separação em camadas de engine/game/render. Nenhum código-fonte do curso
foi copiado; a implementação (classes, algoritmos, shaders) é original.
