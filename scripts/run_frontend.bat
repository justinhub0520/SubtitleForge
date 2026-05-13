@echo off
echo ========================================
echo  SubForge Frontend - Deploy & Run
echo ========================================

set QT6_DIR=F:\QT6\Qt6\6.11.0\mingw_64
set QT6_MINGW=F:\QT6\Qt6\Tools\mingw1310_64
set PATH=%QT6_MINGW%\bin;%QT6_DIR%\bin;%PATH%

cd /d "%~dp0.."
set EXE_DIR=build\frontend\frontend
set EXE_PATH=%EXE_DIR%\subforge_frontend.exe

if not exist "%EXE_PATH%" (
    echo ERROR: subforge_frontend.exe not found!
    echo Please run build_frontend.bat first.
    pause
    exit /b 1
)

echo Step 1: Copying Qt6 DLLs with windeployqt...
windeployqt "%EXE_PATH%" --release --no-translations --no-system-d3d-compiler --no-opengl-sw 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo windeployqt failed with error %ERRORLEVEL%
    echo.
    echo Trying fallback: adding Qt bin to PATH and running directly...
)

echo.
echo Step 2: Starting SubForge...
start "" "%EXE_PATH%"
echo SubForge started!
