#!/bin/bash
set -e

echo "=== Step 1: Stop backend ==="
killall -9 subforge_backend 2>/dev/null || true
sleep 1

echo "=== Step 2: Complete clean rebuild ==="
cd ~/SubForge
rm -rf build/backend
cmake -B build -DBUILD_FRONTEND=OFF
cmake --build build -- -j4

echo "=== Step 3: Verify binary ==="
ls -la build/backend/backend/subforge_backend
strings build/backend/backend/subforge_backend | grep -c "https://dashscope" || true

echo "=== Step 4: Start backend ==="
nohup ./build/backend/backend/subforge_backend > backend.log 2>&1 &
sleep 2

echo "=== Step 5: Health check ==="
wget -q -O- http://localhost:8080/api/health

echo "=== REBUILD COMPLETE ==="