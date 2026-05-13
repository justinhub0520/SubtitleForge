#!/bin/bash
cd /home/project01/SubForge

echo "=== CHECK FILES ==="
ls -la build/ 2>&1
ls -la build/backend/ 2>&1
ls -la build/backend/backend/ 2>&1

echo "=== CMake rebuild ==="
cmake -B build -DBUILD_FRONTEND=OFF 2>&1
cmake --build build -- -j4 2>&1

echo "=== CHECK BINARY ==="
ls -la build/backend/backend/subforge_backend 2>&1

echo "=== START ==="
killall -9 subforge_backend 2>/dev/null
nohup ./build/backend/backend/subforge_backend > backend.log 2>&1 &
sleep 2

echo "=== VERIFY ==="
ps aux | grep subforge_backend | grep -v grep
wget -q -O- http://localhost:8080/api/health 2>&1
echo "DONE"