#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$DIR"

MINGW_ROOT="/tmp/mingw64_root"
CXX="$MINGW_ROOT/usr/bin/x86_64-w64-mingw32-g++"

export PATH="$MINGW_ROOT/usr/bin:$PATH"

INCLUDES="-I$MINGW_ROOT/usr/x86_64-w64-mingw32/sys-root/mingw/include -I/usr/x86_64-w64-mingw32/sys-root/mingw/include -I$DIR/imgui -I$DIR/minhook/include -I$DIR/src"
LIB_PATHS="-B$MINGW_ROOT/usr/libexec/gcc/x86_64-w64-mingw32/16.2.1/ \
           -B$MINGW_ROOT/usr/x86_64-w64-mingw32/bin \
           -B/usr/x86_64-w64-mingw32/sys-root/mingw/lib \
           -L$MINGW_ROOT/usr/lib/gcc/x86_64-w64-mingw32/16.2.1/ \
           -L$MINGW_ROOT/usr/x86_64-w64-mingw32/sys-root/mingw/lib \
           -L/usr/x86_64-w64-mingw32/sys-root/mingw/lib"

STATIC_FLAGS="-static -static-libgcc -static-libstdc++"
LIBS="-ld3d11 -ldxgi -ld3dcompiler -ldwmapi -lgdi32"

CXXFLAGS="-O2 -std=c++17 -w $INCLUDES $LIB_PATHS"

mkdir -p build bin

echo "[*] Compiling MinHook..."
$CXX $CXXFLAGS -c minhook/src/buffer.c -o build/buffer.o
$CXX $CXXFLAGS -c minhook/src/hook.c -o build/hook.o
$CXX $CXXFLAGS -c minhook/src/trampoline.c -o build/trampoline.o
$CXX $CXXFLAGS -c minhook/src/hde/hde64.c -o build/hde64.o

echo "[*] Compiling ImGui..."
$CXX $CXXFLAGS -c imgui/imgui.cpp -o build/imgui.o
$CXX $CXXFLAGS -c imgui/imgui_draw.cpp -o build/imgui_draw.o
$CXX $CXXFLAGS -c imgui/imgui_tables.cpp -o build/imgui_tables.o
$CXX $CXXFLAGS -c imgui/imgui_widgets.cpp -o build/imgui_widgets.o
$CXX $CXXFLAGS -c imgui/backends/imgui_impl_win32.cpp -o build/imgui_impl_win32.o
$CXX $CXXFLAGS -c imgui/backends/imgui_impl_dx11.cpp -o build/imgui_impl_dx11.o

echo "[*] Compiling Cheat Modules..."
$CXX $CXXFLAGS -c src/menu.cpp -o build/menu.o
$CXX $CXXFLAGS -c src/dx11_hook.cpp -o build/dx11_hook.o
$CXX $CXXFLAGS -c src/proxy_dxgi.cpp -o build/proxy_dxgi.o
$CXX $CXXFLAGS -c src/proxy_xinput.cpp -o build/proxy_xinput.o
$CXX $CXXFLAGS -c src/dllmain.cpp -o build/dllmain.o

CORE_OBJS="build/buffer.o build/hook.o build/trampoline.o build/hde64.o \
           build/imgui.o build/imgui_draw.o build/imgui_tables.o build/imgui_widgets.o \
           build/imgui_impl_win32.o build/imgui_impl_dx11.o \
           build/menu.o build/dx11_hook.o build/dllmain.o"

echo "[*] Linking xinput1_3.dll (Recommended for Steam Proton)..."
$CXX -shared $CORE_OBJS build/proxy_xinput.o $LIB_PATHS $STATIC_FLAGS $LIBS -Wl,--subsystem,windows -o bin/xinput1_3.dll

echo "[*] Linking dxgi.dll (Proxy Drop-in DLL)..."
$CXX -shared $CORE_OBJS build/proxy_dxgi.o $LIB_PATHS $STATIC_FLAGS $LIBS -Wl,--subsystem,windows -o bin/dxgi.dll

echo "[*] Linking tjoc_cheat.dll (Standalone Injectable DLL)..."
$CXX -shared $CORE_OBJS $LIB_PATHS $STATIC_FLAGS $LIBS -Wl,--subsystem,windows -o bin/tjoc_cheat.dll

mingw-strip bin/*.dll

echo "[+] Build complete! Generated:"
ls -lh bin/
