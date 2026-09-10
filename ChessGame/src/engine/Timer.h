#pragma once
#include <windows.h>

// Mede o delta time (segundos entre um frame e outro) usando o contador
// de alta resolucao do Windows. Reservado para futuras animacoes de
// movimento de pecas; hoje o jogo eh orientado a eventos de clique.
class Timer {
public:
    Timer();
    float Tick();

private:
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_lastTime;
};
