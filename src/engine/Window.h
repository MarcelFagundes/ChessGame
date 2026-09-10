#pragma once
#include <windows.h>
#include <functional>

// Encapsula a criacao e o loop de mensagens de uma janela Win32.
// Eventos sao expostos como std::function para que o codigo de jogo
// (em main.cpp) reaja a eles sem precisar conhecer detalhes do Win32.
class Window {
public:
    bool Create(int width, int height, const wchar_t* title);
    bool ProcessMessages(); // retorna false quando a janela eh fechada

    HWND GetHandle() const { return m_hwnd; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    std::function<void(int width, int height)> OnResize;
    std::function<void(int x, int y)> OnMouseDown;
    std::function<void(int key)> OnKeyDown;

private:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    // 
    // Dentro da definição da classe Window em Window.h:
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // Adicione o HWND aqui


    HWND m_hwnd = nullptr;
    int m_width = 0;
    int m_height = 0;
};
