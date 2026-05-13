#!/bin/bash
cd /home/project01/SubForge
killall -9 subforge_backend 2>/dev/null
sleep 1

cd /home/project01/SubForge/build
make -j4 2>&1

echo "--- BINARY ---"
ls -la backend/subforge_backend

cd /home/project01/SubForge
nohup ./build/backend/subforge_backend > backend.log 2>&1 &
sleep 2

echo "--- PS ---"
ps aux | grep subforge_backend | grep -v grep

echo "--- LOG ---"
cat backend.log

echo "--- HEALTH ---"
wget -q -O- http://localhost:8080/api/health

echo "DONE"