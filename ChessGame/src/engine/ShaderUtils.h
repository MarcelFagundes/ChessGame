#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <windows.h>

using Microsoft::WRL::ComPtr;

// Compila um arquivo .hlsl em tempo de execucao. Isso facilita iterar em
// shaders durante o aprendizado (edita o .hlsl, roda de novo, sem
// recompilar o projeto inteiro em C++). Em um jogo "de producao" os
// shaders costumam ser pre-compilados (.cso) no processo de build.
inline bool CompileShaderFromFile(const wchar_t* path, const char* entryPoint,
                                   const char* target, ComPtr<ID3DBlob>& outBlob) {
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        path, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint, target, flags, 0, &outBlob, &errorBlob
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        return false;
    }
    return true;
}
