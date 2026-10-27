#!/bin/bash
# TJoC: R Cheat Menu - Cross-Platform Installer

DEFAULT_TARGET="/home/androtech/Downloads/tjoc-r-beta-0.1.0/TJoC- R Beta 0.1.0/WindowsNoEditor/Engine/Binaries/Win64"
TARGET_DIR="${1:-$DEFAULT_TARGET}"
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SRC_XINPUT="$DIR/bin/xinput1_3.dll"
SRC_DXGI="$DIR/bin/dxgi.dll"

if [ ! -d "$TARGET_DIR" ]; then
    echo "[!] Target directory does not exist: $TARGET_DIR"
    echo "Usage: ./install.sh [path/to/TJoC/Engine/Binaries/Win64]"
    echo ""
    echo "Or manually copy bin/xinput1_3.dll into your game's Engine/Binaries/Win64/ directory."
    exit 1
fi

echo "[*] Installing proxy DLLs into: $TARGET_DIR"
cp -v "$SRC_XINPUT" "$TARGET_DIR/xinput1_3.dll"
cp -v "$SRC_DXGI" "$TARGET_DIR/dxgi.dll"

echo ""
echo "[+] Successfully installed cheat menu DLLs!"
echo "[*] Launch the game and press [END] to open the cheat menu in-game."
