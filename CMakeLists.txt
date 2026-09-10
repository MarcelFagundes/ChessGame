cmake_minimum_required(VERSION 3.20)
project(ChessGame CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ChessGame WIN32
    src/main.cpp
    src/engine/Window.cpp
    src/engine/GraphicsDevice.cpp
    src/engine/Timer.cpp
    src/engine/TextAtlas.cpp
    src/game/Board.cpp
    src/game/MoveGenerator.cpp
    src/render/BoardRenderer.cpp
    src/render/PieceRenderer.cpp
)

target_include_directories(ChessGame PRIVATE src)

# Forca o uso das APIs Win32 "wide" (W) em vez das ANSI (A). Sem isso,
# WNDCLASSEX/CreateWindowEx viram suas versoes ANSI e recusam strings
# wchar_t (L"..."), como as usadas em Window.cpp.
# target_compile_definitions(ChessGame PRIVATE UNICODE _UNICODE)

if(MSVC)
    # Ativa o Edit and Continue (/ZI) para compilações de Debug
    target_compile_options(ChessGame PRIVATE 
        $<$<CONFIG:Debug>:/ZI>
    )
    # Garanta também que o Linker use as flags compatíveis com /ZI
    target_link_options(ChessGame PRIVATE 
        $<$<CONFIG:Debug>:/INCREMENTAL>
    ) 
endif()

# Força o compilador a usar as versões Unicode das funções do Windows
if(WIN32)
    add_compile_definitions(UNICODE _UNICODE)
endif()

target_link_libraries(ChessGame PRIVATE
    d3d11
    dxgi
    d3dcompiler
    user32
    gdi32
)

# Os shaders (.hlsl) sao compilados em tempo de execucao (D3DCompileFromFile),
# entao precisam existir ao lado do executavel. Este comando copia a pasta
# Shaders automaticamente a cada build.
add_custom_command(TARGET ChessGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/render/Shaders
        $<TARGET_FILE_DIR:ChessGame>/Shaders
)