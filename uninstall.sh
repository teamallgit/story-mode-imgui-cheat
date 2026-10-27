#!/bin/bash
GAME_DIR="/home/androtech/Downloads/tjoc-r-beta-0.1.0/TJoC- R Beta 0.1.0/WindowsNoEditor/Engine/Binaries/Win64"
TARGET="$GAME_DIR/dxgi.dll"

if [ -f "$TARGET" ]; then
    rm -v "$TARGET"
    echo "[+] Successfully uninstalled cheat menu (removed dxgi.dll from game folder)."
else
    echo "[*] No dxgi.dll found in game folder."
fi
