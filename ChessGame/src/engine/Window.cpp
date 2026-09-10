#include "Window.h"
#include <string>

// Funcao exigida pela API do Windows: assinatura fixa, por isso eh estatica.
// Ela recupera o ponteiro "this" (salvo em GWLP_USERDATA na criacao da janela)
// e repassa a mensagem para o metodo de instancia HandleMessage.

LRESULT CALLBACK Window::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Window* self = nullptr;

    if (msg == WM_NCCREATE) {
        // 1. Mudado para CREATESTRUCTW devido ao uso do Unicode
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        if (cs != nullptr) {
            self = static_cast<Window*>(cs->lpCreateParams);
            // 2. Garante que a variável interna m_hwnd do objeto já seja salva aqui
            if (self != nullptr) {
                self->m_hwnd = hwnd;
            }
            // 3. Forçado o uso da versão Unicode (SetWindowLongPtrW)
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        }
    }
    else {
        // 4. Forçado o uso da versão Unicode (GetWindowLongPtrW)
        self = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    // Passa o 'hwnd' atual como parâmetro para o HandleMessage trabalhar com segurança
    if (self) return self->HandleMessage(hwnd, msg, wParam, lParam);

    // 5. Forçado o uso da versão Unicode (DefWindowProcW)
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// AJUSTE: Adicionado o parâmetro 'HWND hwnd' na assinatura
LRESULT Window::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        m_width = LOWORD(lParam);
        m_height = HIWORD(lParam);
        if (OnResize) OnResize(m_width, m_height);
        return 0;

    case WM_LBUTTONDOWN:
        if (OnMouseDown) OnMouseDown(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_KEYDOWN:
        if (OnKeyDown) OnKeyDown(static_cast<int>(wParam));
        return 0;

    default:
        // CORREÇÃO CRÍTICA: Passando 'hwnd' (garantido) em vez de 'm_hwnd' (que podia estar nulo)
        // Também alterado para DefWindowProcW (Unicode)
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

bool Window::Create(int width, int height, const wchar_t* title) {
    m_width = width;
    m_height = height;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"ChessGameWindowClass";

    if (!RegisterClassExW(&wc)) {
        DWORD erro = GetLastError();
        if (erro != 1410) {
            // Criando a string de forma limpa para não errar os parênteses da função
            std::string msgErro = "Erro no RegisterClassExW. Codigo: " + std::to_string(erro);
            MessageBoxA(nullptr, msgErro.c_str(), "Diagnostico Window", MB_ICONERROR);
            return false;
        }
    }

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowExW(
        0,
        L"ChessGameWindowClass",
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, GetModuleHandle(nullptr), this
    );

    if (!m_hwnd) {
        DWORD erro = GetLastError();
        // Criando a string de forma limpa para passar os 4 argumentos corretos
        std::string msgErro = "Erro no CreateWindowExW. Codigo: " + std::to_string(erro);
        MessageBoxA(nullptr, msgErro.c_str(), "Diagnostico Window", MB_ICONERROR);
        return false;
    }

    ShowWindow(m_hwnd, SW_SHOW);
    return true;
}

bool Window::ProcessMessages() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) return false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}