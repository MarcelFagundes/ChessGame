#include "Timer.h"

Timer::Timer() {
    QueryPerformanceFrequency(&m_frequency);
    QueryPerformanceCounter(&m_lastTime);
}

float Timer::Tick() {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    float delta = static_cast<float>(now.QuadPart - m_lastTime.QuadPart) /
                  static_cast<float>(m_frequency.QuadPart);
    m_lastTime = now;
    return delta;
}
