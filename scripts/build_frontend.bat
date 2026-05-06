@echo off
echo Building SubForge frontend...

cd /d "%~dp0.."

if not exist build\frontend mkdir build\frontend
cd build\frontend

cmake ../.. -DBUILD_BACKEND=OFF -DBUILD_FRONTEND=ON
cmake --build . --config Release

echo Frontend built successfully.
echo Run with: Release\subforge_frontend.exe