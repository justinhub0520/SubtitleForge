#!/bin/bash
killall -9 subforge_backend 2>/dev/null
cd ~/SubForge
rm -rf build/backend/backend/CMakeFiles
cmake -B build -DBUILD_FRONTEND=OFF
cmake --build build -- -j4
echo "=== BUILD_STATUS ==="
echo $?
echo "=== CURL_LINK ==="
ldd build/backend/backend/subforge_backend | grep curl
echo "=== STARTING ==="
nohup ./build/backend/backend/subforge_backend > backend.log 2>&1 &
sleep 2
ps aux | grep subforge_backend | grep -v grep
echo "=== HEALTH ==="
wget -q -O- http://localhost:8080/api/health