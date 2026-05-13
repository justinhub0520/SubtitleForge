@echo off
echo ========================================
echo Building SubForge Frontend with Qt6
echo ========================================

REM Set Qt6 MinGW 64-bit as primary compiler
set QT6_DIR=F:\QT6\Qt6\6.11.0\mingw_64
set QT6_MINGW=F:\QT6\Qt6\Tools\mingw1310_64
set PATH=%QT6_MINGW%\bin;%QT6_DIR%\bin;%PATH%

echo Using Qt6 from: %QT6_DIR%
echo Using MinGW from: %QT6_MINGW%

cd /d "%~dp0.."

REM Clean previous build
if exist build\frontend (
    echo Cleaning previous build...
    rmdir /s /q build\frontend
)
mkdir build\frontend
cd build\frontend

echo Configuring with CMake...
cmake ../.. ^
    -DBUILD_BACKEND=OFF ^
    -DBUILD_FRONTEND=ON ^
    -DCMAKE_PREFIX_PATH="%QT6_DIR%" ^
    -DCMAKE_C_COMPILER="%QT6_MINGW%/bin/gcc.exe" ^
    -DCMAKE_CXX_COMPILER="%QT6_MINGW%/bin/g++.exe" ^
    -DCMAKE_MAKE_PROGRAM="%QT6_MINGW%/bin/mingw32-make.exe" ^
    -G "MinGW Makefiles"

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building...
mingw32-make -j4

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Frontend built successfully!
echo Executable: build\frontend\subforge_frontend.exe
echo ========================================
pause
