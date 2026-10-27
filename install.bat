@echo off
setlocal

echo ========================================================
echo The Joy of Creation: Reborn - Cheat Menu Installer
echo ========================================================
echo.

set "TARGET_DIR=%~1"
if "%TARGET_DIR%"=="" (
    if exist "Engine\Binaries\Win64" (
        set "TARGET_DIR=Engine\Binaries\Win64"
    ) else if exist "..\Engine\Binaries\Win64" (
        set "TARGET_DIR=..\Engine\Binaries\Win64"
    )
)

if "%TARGET_DIR%"=="" (
    echo [!] Could not locate Engine\Binaries\Win64 automatically.
    echo.
    echo Manual Installation:
    echo Simply copy bin\xinput1_3.dll into your game's:
    echo   [Game Folder]\Engine\Binaries\Win64\xinput1_3.dll
    echo.
    set /p "TARGET_DIR=Enter full path to Engine\Binaries\Win64 folder: "
)

if exist "%TARGET_DIR%" (
    echo [*] Copying xinput1_3.dll to: %TARGET_DIR%
    copy /Y "%~dp0bin\xinput1_3.dll" "%TARGET_DIR%\xinput1_3.dll"
    echo.
    echo [+] Successfully installed!
    echo [*] Start the game and press [END] to open the cheat menu.
) else (
    echo [!] Error: Directory "%TARGET_DIR%" does not exist.
)

echo.
pause
